#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h> 
#include <unistd.h>

int receive_image(int socket)
{ 
    // Start function

    int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;

    char imagearray[10241],verify = '1';
    FILE *image;

    //Find the size of the image
    do
    {
        stat = read(socket, &size, sizeof(int));
    }
    while(stat<0);

    printf("Packet received.\n");
    printf("Packet size: %i\n",stat);
    printf("Image size: %i\n",size);
    printf(" \n");

    char buffer[] = "Got it";

    //Send our verification signal
    do
    {
        stat = write(socket, &buffer, sizeof(int));
    }
    while(stat < 0);

    printf("Reply sent\n");
    printf(" \n");

    image = fopen("/home/dotai/Pictures/image_copy.png", "w");

    if( image == NULL) 
    {
        printf("Error has occurred. Image file could not be opened\n");
        return -1; 
    }

    //Loop while we have not received the entire file yet


    int need_exit = 0;
    struct timeval timeout = {10,0};

    fd_set fds;
    int buffer_fd, buffer_out;

    while(recv_size < size) 
    {
        //while(packet_index < 2){

        FD_ZERO(&fds);
        FD_SET(socket,&fds);

        buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

        if (buffer_fd < 0)
           printf("error: bad file descriptor set.\n");

        if (buffer_fd == 0)
           printf("error: buffer read timeout expired.\n");

        if (buffer_fd > 0)
        {
            do
            {
                read_size = read(socket,imagearray, 10241);
            }
            while(read_size <0);

            printf("Packet number received: %i\n",packet_index);
            printf("Packet size: %i\n",read_size);


            //Write the currently read data into our image file
            write_size = fwrite(imagearray,1,read_size, image);
            printf("Written image size: %i\n",write_size);

            if(read_size !=write_size) 
            {
                printf("error in read write\n");    
            }


             //Increment the total number of bytes read
            recv_size += read_size;
            packet_index++;
            printf("Total received image size: %i\n",recv_size);
            printf(" \n");
            printf(" \n");
        }
    }


    fclose(image);
    printf("Image successfully Received!\n");
    return 1;
}

int main(int argc, char *argv[])
{
    int sock_server, sock_client;
    struct sockaddr_in serverAddress, clientAddress;
    int len;

    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_server < 0)
    {
        fprintf(stderr, "Cannot create socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Create socket successfully\n");
    }

    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(1234);

    if (bind(sock_server, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        fprintf(stderr, "Bind Failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Bind Done\n");
    }

    if (listen(sock_server, 3) != 0)
    {
        fprintf(stderr, "Listen Failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Listenning\n");
    }

    while (1)
    {
        printf("Waiting for incoming connections\n");
        len = sizeof (struct sockaddr_in);

        sock_client = accept(sock_server, (struct sockaddr *)&clientAddress, (socklen_t *)&len);
        if (sock_client < 0)
        {
            fprintf(stderr, "Cannot accept client\n");
            continue;
        }
        else
        {
            printf("Connect client: %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        }

        receive_image(sock_client);
    }
    close(sock_server);
    exit(EXIT_SUCCESS);
}