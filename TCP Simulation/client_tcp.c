#include "tcp.h"

#define CLIENT_PORT_PATH "clientSide/ports/"
#define SYSTEM_FIFO_PATH "clientSide/system.fifo"

connectionList* head = NULL;
sem_t lock;

// File Descriptors of Network and System
int network_fd[2];
int system_fd;

connectionList* getConnections(uint16_t sourcePort, uint16_t destinationPort){
    if(head == NULL){
        head = (connectionList*)malloc(sizeof(connectionList));
        head->source_port = sourcePort;
        head->dest_port = destinationPort;
        head->connectionStatus = REQ;
        return head;
    }
    else{
        connectionList* curr = head;
        connectionList* prev = NULL;
        while(curr){
            if(curr->source_port == sourcePort && curr->dest_port == destinationPort){
                return curr;
            }
            prev = curr;
            curr = curr->next;
        }
        // printf("New Connection\n");
        // Adding new node at last and returning it for the handler to update the details
        connectionList* newNode = (connectionList*)malloc(sizeof(connectionList));
        newNode->source_port = sourcePort;
        newNode->dest_port = destinationPort;
        newNode->connectionStatus = REQ;
        prev->next = newNode;
        return newNode;
    }
}

int hanshake(uint16_t sourcePort, uint16_t destinationPort){
    /*
        -> handshake() performs 3-way handshaking for requested     connection on (sourcePort,destPort).
        -> Returns either EST(0) connection or RST(1) connection in case no passive open on Destination Port.
    */

    tcp_header syn_packet, ack_syn_packet, ack_packet;
    
    // Received a connection request from client process
    // Assign a random sequence number

    int client_seqnum = rand();

    // Make SYN segment
    syn_packet = create_tcp_header(sourcePort, destinationPort, client_seqnum, 0, SYN_PACKET, 0, 0, 0);
    // Send Packet
    send_packet(network_fd[1], &syn_packet, sizeof(syn_packet));
    printf("SYN_PACKET Sent!\n");
    printTcpHeader(&syn_packet);



    // Receive ACK+SYN segment
    receive_packet(network_fd[0], &ack_syn_packet, sizeof(ack_syn_packet));
    if(ack_syn_packet.data_offset_flags == ACK_SYN_PACKET){
        printf("Received ACK+SYN PACKET\n");
        printTcpHeader(&ack_syn_packet);

        if(ack_syn_packet.ack_num == syn_packet.sequence_num + 1){
            // Opent Connection for Client
            // Print ACK received for the Client.
            // Send ACK packet for server side SYN request
            copyTcpHeader(&ack_packet, &ack_syn_packet);
            ack_packet.sequence_num = syn_packet.sequence_num;
            ack_packet.ack_num = ack_syn_packet.sequence_num + 1;
            ack_packet.data_offset_flags = ACK_PACKET;
        }
    }
    else{
        // Other than ACK
        printf("Received RST segment");
        if(ack_syn_packet.data_offset_flags == RST_PACKET){
            printf("No Server Application at Destination Port: %d", destinationPort);
            return 1;
        }
    }
    

    // Send ACK packet
    printf("Sent ACK PACKET\n");
    printTcpHeader(&ack_packet);
    send_packet(network_fd[1], &ack_packet, sizeof(ack_packet));

    return 0;
}

void handleClient(void){
    printf("Handling Clients.\n");
    while(true){
        clientReqPacket* reqPacket = (clientReqPacket*)malloc(sizeof(clientReqPacket));
        
        // Read returns data on the FIFO, if no data but write ends are open then shall block the process until any data is written on FIFO.
        ssize_t readBytes =  Read(system_fd, reqPacket, sizeof(clientReqPacket));
        if(readBytes == 0){
            // If all write ends are closed (No client Process), 'read' returns 0 -> EOF.
            printf("No Active Clients.\n");
            close(system_fd);
            // Close the FIFO read end.
            // Go back to main function and Open Read end again. Open will again block the process execution until a client process open FIFO for Write.
            return;
        }
        printf("Received:\n");
        print_clientReqPacket(reqPacket);

        // Get Records of Current Request, if not then create new.
        connectionList* activeReq = getConnections(reqPacket->source_port, reqPacket->dest_port);
        // Printing Connection Record
        print_connectionList(activeReq);
        
        // Perform action based on request received.
        if(reqPacket->isData == false){
            printf("Connection Control..\n");
            // Client Process can either request to OPEN connection or CLOSE connection
            if(reqPacket->request == REQ){
                // Open FIFO end for Writing, store the file descriptor in 'connectionList->portFd'
                // Perform Handshake
                // Relay connection status back to client process.
                printf("New Connection Request received at\nSource Port: %d, Destination Port: %d\n", reqPacket->source_port, reqPacket->dest_port);
                // Opening Write End for the port
                char fifoPath[50];
                sprintf(fifoPath, "%s%d.fifo", CLIENT_PORT_PATH, reqPacket->source_port);
                activeReq->portFd = Open(fifoPath, (O_EXCL | O_WRONLY));
                // Perform Handshaking
                int receivedFlag = hanshake(reqPacket->source_port, reqPacket->dest_port);
                printf("Received Packet: %d\n", receivedFlag);
                tcpClientPacket* toClient = (tcpClientPacket*)malloc(sizeof(tcpClientPacket));
                if(receivedFlag == 0){
                    printf("Connection: EST\n");
                    activeReq->connectionStatus = EST;
                    toClient->info = EST;
                    toClient->isData = false;
                }
                else{
                    printf("Connection: RST\n");
                    activeReq->connectionStatus = RST;
                    toClient->isData = false;
                    toClient->info = RST;
                }
                printf("Client Packet sent back to client\n");
                print_tcpClientPacket(toClient);
                Write(activeReq->portFd, toClient, sizeof(tcpClientPacket));
                free(toClient);
            }
            else if(reqPacket->request == FIN){
                // TERMINATE Connection
            }   
        }
        else{
            tcp_header* dataSegment = (tcp_header*)malloc(sizeof(tcp_header));
            if(activeReq->connectionStatus == EST){
                // Transfer Data
                if(reqPacket->isData){
                    dataSegment->source_port = reqPacket->source_port;
                    dataSegment->dest_port = reqPacket->dest_port;
                    memset(dataSegment->data, 0, BUFSIZ);
                    strcpy(dataSegment->data, reqPacket->data);
                    send_packet(network_fd[1], dataSegment, sizeof(tcp_header));
                    memset(dataSegment, 0, sizeof(tcp_header));
                }

                // Waiting for response
                tcpClientPacket* sendData = (tcpClientPacket*)malloc(sizeof(tcpClientPacket));
                readBytes = receive_packet(network_fd[0], dataSegment, sizeof(tcp_header));
                memset(sendData->data, 0, BUFSIZ);
                if(readBytes > 20){
                    sendData->isData = true;
                    strcpy(sendData->data, dataSegment->data);
                    Write(activeReq->portFd, sendData, sizeof(tcpClientPacket));
                }
                free(sendData);
            }
            else if(activeReq->connectionStatus == CLOSED){
                // Closed Connection
            }
            else if(activeReq->connectionStatus == REQ){
                // Redirect for hanshake
            }
            free(dataSegment);
        }
    }
}

int main(){
    // For random sequence number generation in 'handshake' function.
    srand(time(NULL));
    
    // Creating two way 'Network' FIFO, emulating NETWORK
    // BOTH client TCP and server TCP will communicate through NETWORK
    twoWayFifo(network_fd, NETWORK_FIFO_PATH, O_EXCL, true);


    // Creating system fifo, all process must send requests to TCP via system.fifo
    while(true){
        printf("Waiting for Client Processes\n");
        // For client to TCP_client communication, we have 'system.fifo'
        // TCP will communicate via source port mentioned by client process in their request packets.
        system_fd = Open(SYSTEM_FIFO_PATH, (O_EXCL | O_RDONLY));
        // Will handle client only if there are active client process seeking connections
        handleClient();
    }


    // Closing System FIFO
    Close(system_fd);
    // Closing Network FIFO
    closeTwoWayFifo(network_fd);
    return 0;
}