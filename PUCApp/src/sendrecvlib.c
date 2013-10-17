#include "sendrecvlib.h"

asynUser *user_sg;
asynUser *user_curve;
asynUser *user_frontend;
/*
 * Send command and get reply
 */
int sendCommandEPICSsg(uint8_t *data, uint32_t *count)
{
    size_t wrote;

    asynStatus status = pasynOctetSyncIO->write(user_sg,(char *)data,*count,5000,&wrote);

    if(status == asynSuccess) return EXIT_SUCCESS;

    return EXIT_FAILURE;
}

int recvCommandEPICSsg(uint8_t *data, uint32_t *count)
{
    #ifdef DEBUG
    printf("recv\n");
    #endif

    asynStatus status;
    int eomReason;
    size_t bread;
    uint8_t packet[17000];
    size_t size;
    int err;

    #ifdef PUC
    //Read address
    uint8_t * address;
    address = (uint8_t*) malloc(2*sizeof(char));

    status = pasynOctetSyncIO->read(user_sg, (char*) address, 2, 5000, &bread, &eomReason);
    printf("Address = %02X \n", address[0]);

    if(address[0] != 0x00) return EXIT_FAILURE;
    #endif

    uint8_t* header;
    header = (uint8_t*) malloc(2*sizeof(char));

    status = pasynOctetSyncIO->read(user_sg, (char*) header, 2, 5000, &bread, &eomReason);
    if(err = (status != asynSuccess)) printf("Error %d reading header\n", err); //TODO: Return error;

    if(header[1] == 255)

        size = SLLP_CURVE_BLOCK_PKT;
    else
        size = header[1];

    uint8_t* payload;

    if(size > 0)
    {
        payload = (uint8_t*) malloc(size*sizeof(char));
        status = pasynOctetSyncIO->read(user_sg, (char*) payload, size, 5000, &bread, &eomReason);
        if(err = (status != asynSuccess)) printf("Error %d reading payload\n", err);
    }

    memcpy(packet, header, 2);
    if(size > 0) memcpy(packet+2, payload, size);

    *count = size+2;

    memcpy(data, packet, *count);

    free(header);
    if(size > 0) free(payload);


    #ifdef PUC
    uint8_t* checksum;
    checksum = (uint8_t*) malloc(1*sizeof(char));
    status = pasynOctetSyncIO->read(user_sg, (char*)checksum, 1, 5000, &bread, &eomReason);
    //TODO: Verify checksum
    #endif

    return EXIT_SUCCESS;
}
int sendCommandEPICScurve(uint8_t *data, uint32_t *count)
{
    size_t wrote;

    asynStatus status = pasynOctetSyncIO->write(user_curve,(char *)data,*count,5000,&wrote);

    if(status == asynSuccess) return EXIT_SUCCESS;

    return EXIT_FAILURE;
}

int recvCommandEPICScurve(uint8_t *data, uint32_t *count)
{
    #ifdef DEBUG
    printf("recv\n");
    #endif

    asynStatus status;
    int eomReason;
    size_t bread;
    uint8_t packet[17000];
    size_t size;
    int err;

    #ifdef PUC
    //Read address
    uint8_t * address;
    address = (uint8_t*) malloc(2*sizeof(char));

    status = pasynOctetSyncIO->read(user_curve, (char*) address, 2, 5000, &bread, &eomReason);
    printf("Address = %02X \n", address[0]);

    if(address[0] != 0x00) return EXIT_FAILURE;
    #endif

    uint8_t* header;
    header = (uint8_t*) malloc(2*sizeof(char));

    status = pasynOctetSyncIO->read(user_curve, (char*) header, 2, 5000, &bread, &eomReason);
    if(err = (status != asynSuccess)) printf("Error %d reading header\n", err); //TODO: Return error;

    if(header[1] == 255)

        size = SLLP_CURVE_BLOCK_PKT;
    else
        size = header[1];

    uint8_t* payload;

    if(size > 0)
    {
        payload = (uint8_t*) malloc(size*sizeof(char));
        status = pasynOctetSyncIO->read(user_curve, (char*) payload, size, 5000, &bread, &eomReason);
        if(err = (status != asynSuccess)) printf("Error %d reading payload\n", err);
    }

    memcpy(packet, header, 2);
    if(size > 0) memcpy(packet+2, payload, size);

    *count = size+2;

    memcpy(data, packet, *count);

    free(header);
    if(size > 0) free(payload);


    #ifdef PUC
    uint8_t* checksum;
    checksum = (uint8_t*) malloc(1*sizeof(char));
    status = pasynOctetSyncIO->read(user_curve, (char*)checksum, 1, 5000, &bread, &eomReason);
    //TODO: Verify checksum
    #endif

    return EXIT_SUCCESS;
}
int sendCommandEPICSfrontend(uint8_t *data, uint32_t *count)
{
    size_t wrote;

    asynStatus status = pasynOctetSyncIO->write(user_frontend,(char *)data,*count,5000,&wrote);

    if(status == asynSuccess) return EXIT_SUCCESS;

    return EXIT_FAILURE;
}

int recvCommandEPICSfrontend(uint8_t *data, uint32_t *count)
{
    #ifdef DEBUG
    printf("recv\n");
    #endif

    asynStatus status;
    int eomReason;
    size_t bread;
    uint8_t packet[17000];
    size_t size;
    int err;

    #ifdef PUC
    //Read address
    uint8_t * address;
    address = (uint8_t*) malloc(2*sizeof(char));

    status = pasynOctetSyncIO->read(user_frontend, (char*) address, 2, 5000, &bread, &eomReason);
    printf("Address = %02X \n", address[0]);

    if(address[0] != 0x00) return EXIT_FAILURE;
    #endif

    uint8_t* header;
    header = (uint8_t*) malloc(2*sizeof(char));

    status = pasynOctetSyncIO->read(user_frontend, (char*) header, 2, 5000, &bread, &eomReason);
    if(err = (status != asynSuccess)) printf("Error %d reading header\n", err); //TODO: Return error;

    if(header[1] == 255)

        size = SLLP_CURVE_BLOCK_PKT;
    else
        size = header[1];

    uint8_t* payload;

    if(size > 0)
    {
        payload = (uint8_t*) malloc(size*sizeof(char));
        status = pasynOctetSyncIO->read(user_frontend, (char*) payload, size, 5000, &bread, &eomReason);
        if(err = (status != asynSuccess)) printf("Error %d reading payload\n", err);
    }

    memcpy(packet, header, 2);
    if(size > 0) memcpy(packet+2, payload, size);

    *count = size+2;

    memcpy(data, packet, *count);

    free(header);
    if(size > 0) free(payload);


    #ifdef PUC
    uint8_t* checksum;
    checksum = (uint8_t*) malloc(1*sizeof(char));
    status = pasynOctetSyncIO->read(user_frontend, (char*)checksum, 1, 5000, &bread, &eomReason);
    //TODO: Verify checksum
    #endif

    return EXIT_SUCCESS;
}
uint8_t lastCommand;
int sendCommandtest(uint8_t *data, uint32_t *count)
{
        #ifdef DEBUG
        printf("sendCommandtest");
        #endif
        if(*count < 256 && *count > 0)
        {
                int i;
                printf("Count = %d\n", *count);

                for(i = 0; i < *count; i++)
                {
                        printf("data[%d] = %02X\n", i, data[i]);
                }

                lastCommand = data[0];
        }

        return EXIT_SUCCESS;
}
int receiveCommandtest(uint8_t *data, uint32_t *count)
{
        #ifdef DEBUG
        printf("RECEIVE BEGIN\n");
        #endif
        uint8_t packet[8];
        //bool flag = false;

        //Simula uma variavel de leitura de tamanho 3 e uma variavel de escrita de tamanho 3

        if(lastCommand == 0x02)
        {
            printf("Comando de listar variaveis\n");
            packet[0] = 0x03;
            packet[1] = 0x06;
            packet[2] = 0x83;
            packet[3] = 0x03;
            packet[4] = 0x03;
            packet[5] = 0x03;
            packet[6] = 0x03;
            packet[7] = 0x83;
            *count = 8;
        }

        //Read simulation
        else if(lastCommand == 0x10)
        {
                //flag = true;
                printf("Comando de ler variavel\n");
                packet[0] = 0x11;
                packet[1] = 0x03;
                packet[2] = 0x02;
                packet[3] = 0xFF;
                packet[4] = 0x0A;
                *count = 5;
        }

        else if (lastCommand == 0x04)
        {
                //flag = true;
                printf("Comando de listar grupos de variaveis\n");
                packet[0] = 0x05;
                packet[1] = 0x00;
                *count = 2;
        }

        else if(lastCommand == 0x08)
        {
                //flag = true;
                printf("Comando de listar curvas\n");

                packet[0] = 0x09;
                packet[1] = 0x00;
                *count = 2;
        }

        //if(flag) memcpy(data, packet, *count);
        memcpy(data, packet, *count);
        #ifdef DEBUG
        printf("RECEIVE END\n");
        #endif

        return EXIT_SUCCESS;
}
void setEpicsuserfpgasg(asynUser *us){
	user_sg = us;
	return;
}
void setEpicsuserfrontend(asynUser *us){
	user_frontend = us;
	return;
}
void setEpicsuserfpgacurve(asynUser *us){
	user_curve = us;
	return;
}
