#include "tcp.h"

#define SERVER_PORT_PATH "serverSide/ports/"
#define SYSTEM_FIFO_PATH "serverSide/system.fifo"

// Record List
connectionList* head = NULL;

// 'Network' File Descriptor
int network_fd[2];
int system_fd;

bool checkPassiveOpen(connectionList* current){
    printf("Checking for Passive open at PORT: %d\n", current->dest_port);
    char fifoPath[50];
    sprintf(fifoPath, "%s%d.fifo", SERVER_PORT_PATH, current->dest_port);
    if( (current->portFd = Open(fifoPath, (O_EXCL | O_WRONLY))) == -1){
        if(errno = 2){
            // If no such file exist that implies that no server process has opened it.
            // Hence no server process is listening to PORT. Thus no passive open.
            printf("No Passive open found at PORT: %d\n", current->dest_port);
            return false;
        }
    }
    printf("File Descriptor: %d\n", current->portFd);
    printf("System File descriptor: %d\n", system_fd);
    // If exists then portFd has the Write file descriptor.
    printf("Found Passive Open at PORT: %d\n", current->dest_port);
    return true;
}

// Get connection history function
connectionList* getConnections(uint16_t sourcePort, uint16_t destinationPort){
    printf("Fetching Connection details\n");
    if(head == NULL){
        printf("Connection Table Empty...\n");
        head = (connectionList*)malloc(sizeof(connectionList));
        head->source_port = sourcePort;
        head->dest_port = destinationPort;
        head->connectionStatus = IDLE;
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
        printf("No existing record found, creating new record\n");
        connectionList* newNode = (connectionList*)malloc(sizeof(connectionList));
        newNode->source_port = sourcePort;
        newNode->dest_port = destinationPort;
        newNode->connectionStatus = IDLE;
        prev->next = newNode;
        return newNode;
    }
}

// Performs Handshake for the given SYN PACKET
int handshake(tcp_header* syn_packet){
    printf("Performing Connection\n");
    tcp_header ack_syn_packet, ack_packet;
    // Register Connection
    // Add Sequence number, src_port, dest_port
    // Send ACK and SYN segment
    int server_seqnum = rand();
    ack_syn_packet = create_tcp_header(syn_packet->dest_port, syn_packet->source_port, server_seqnum, (syn_packet->sequence_num + 1), ACK_SYN_PACKET, 0, 0, 0);
    printf("Sending ACK+SYN packet\n");
    printTcpHeader(&ack_syn_packet);
    send_packet(network_fd[1], &ack_syn_packet, sizeof(ack_syn_packet));

    // Receive ACK segment for the SYN request
    // Open Connection.

    ssize_t receivedPacketSize = receive_packet(network_fd[0], &ack_packet, sizeof(ack_packet));
    if(receivedPacketSize != sizeof(ack_packet)){
        printf("Packet Lost");
        return 0;
    }
    if(ack_packet.data_offset_flags == ACK_PACKET){
        printf("Received ACK packet\n");
        printTcpHeader(&ack_packet);
        printf("ACK received for Connection with SEQ NUM: %d\n", ack_packet.sequence_num);
    }
    return 1;
}

void handleClientReq(void){
    printf("Handling Client\n");
    while(true){
        tcp_header* rxPacket = (tcp_header*)malloc(sizeof(tcp_header));
        // Listen for incoming requests. Blocking read operation on FIFO
        ssize_t receivedPacketSize = receive_packet(network_fd[0], rxPacket, sizeof(tcp_header));
        printf("Received:\n");
        printTcpHeader(rxPacket);
        // Checking for the record the connection.
        connectionList* currentConnection = getConnections(rxPacket->source_port, rxPacket->dest_port);
        printf("Connection Record Obtained...\n");
        print_connectionList(currentConnection);
        if(currentConnection->connectionStatus == IDLE){
            // Check for Passive open on the received Destination Port from Client Side.
            
            if(checkPassiveOpen(currentConnection) && rxPacket->data_offset_flags == SYN_PACKET){
                // Proceed for Handshake
                if(handshake(rxPacket)){
                    // If ACK received for ACK+SYN packet, Open connection
                    printf("Connection Established for SRC: %d\t DST: %d\n",currentConnection->source_port, currentConnection->dest_port);
                    currentConnection->connectionStatus = EST;
                    tcpClientPacket *serverPacket = (tcpClientPacket*)malloc(sizeof(tcpClientPacket));
                    serverPacket->isData = false;
                    serverPacket->info = EST;
                    // Sending Connection Packet to listening process.
                    printf("Informing the Server Application at PORT: %d\n", currentConnection->dest_port);   
                    print_tcpClientPacket(serverPacket);
                    Write(currentConnection->portFd, serverPacket, sizeof(tcpClientPacket));
                    free(serverPacket);
                }
                else{
                    printf("Unable to handshake.\n");
                    currentConnection->connectionStatus = IDLE;
                }
            }
            else{
                // Send RST PACKET
                printf("No Passive Open at PORT: %d, Sending RST Packet\n", currentConnection->dest_port);
                currentConnection->connectionStatus = RST;
                tcp_header rst_packet;
                rst_packet = create_tcp_header(rxPacket->dest_port, rxPacket->source_port, 0, rxPacket->sequence_num + 1, RST_PACKET, 0, 0, 0);
                send_packet(network_fd[1], &rst_packet, sizeof(tcp_header));
            }
        }
        else if(currentConnection->connectionStatus == EST){
            if(receivedPacketSize > 20){
                // Contains Data
                printf("Data Segment Received.\n");
                tcpClientPacket* serverPacket = (tcpClientPacket*)malloc(sizeof(tcpClientPacket));
                serverPacket->info = EST;
                serverPacket->isData = true;
                memset(serverPacket->data, 0, BUFSIZ);
                strcpy(serverPacket->data, rxPacket->data);
                print_tcpClientPacket(serverPacket);
                // Send Client Req
                printf("Relaying it to Server Application at %d\n", currentConnection->dest_port);
                Write(currentConnection->portFd, serverPacket, sizeof(tcpClientPacket));
    
                memset(serverPacket, 0, sizeof(tcpClientPacket));
                
                // Get response
                printf("Waiting for Server response.....\n");
                Read(system_fd, serverPacket, sizeof(tcpClientPacket));
                
                tcp_header* dataSegment = (tcp_header*)malloc(sizeof(tcp_header));
                
                if(serverPacket->isData){
                    copyTcpHeader(dataSegment, rxPacket);
                    memset(dataSegment->data, 0, BUFSIZ);
                    strcpy(dataSegment->data, serverPacket->data);
                    printf("Sending the data back to client.\n");
                    send_packet(network_fd[1], dataSegment, sizeof(tcp_header));
                }
                free(dataSegment);
                free(serverPacket);
            }
            else if(rxPacket->data_offset_flags == FIN_PACKET){
                // Terminate Connection.
            }
        }
        free(rxPacket);
    }
}

int main(){
    srand(time(NULL));
    // Creating two way 'Network' FIFO, emulating NETWORK
    // BOTH client TCP and server TCP will communicate through NETWORK
    twoWayFifo(network_fd, NETWORK_FIFO_PATH, O_EXCL, false);
    
    while(true){
        printf("Waiting for Server Applications...\n");
        system_fd = Open(SYSTEM_FIFO_PATH, (O_EXCL | O_RDONLY));
        handleClientReq();
    }

    closeTwoWayFifo(network_fd);
    return 0;
}