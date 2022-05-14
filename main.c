#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <math.h>
#include <time.h>
#include "main.h"

#define BROADCAST_IP "GET BROADCAST IP "
#define NETWORK_NUMBER "GET NETWORK NUMBER IP "
#define HOSTS_RANGE "GET HOSTS RANGE IP "
#define RANDOM_SUBNETS "GET RANDOM SUBNETS NETWORK NUMBER "
#define REGEX_IP_MASK "[0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9] \
MASK ([0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9]|/[0-9]?[0-9])"
#define RANDOM_SUBNETS_REGEX " NUMBER [0-9]?[0-9]?[0-9]?[0-9]?[0-9]?[0-9]?[0-9]?[0-9]?[0-9]?[0-9] SIZE ([0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9]|/[0-9]?[0-9])"
#define MAX_SUBNET 4194304

void short_mask_to_dotted_mask2(struct Subnet_data *subnet_datas, int mask){
    unsigned int shift = 32 - mask;
    unsigned char full_mask[4] = {1, 1, 1, 1};

    if (shift < 8){
        subnet_datas->size[3] = ((full_mask[3] << shift)-1) ^ 255;
        subnet_datas->size[2] = 255;
        subnet_datas->size[1] = 255;
        subnet_datas->size[0] = 255;
    }
    else if (shift < 16){
        subnet_datas->size[3] = 0;
        subnet_datas->size[2] = (full_mask[2] << shift-8)-1 ^ 255;
        subnet_datas->size[1] = 255;
        subnet_datas->size[0] = 255;
    }
    else if (shift < 24){
        subnet_datas->size[3] = 0;
        subnet_datas->size[2] = 0;
        subnet_datas->size[1] = (full_mask[1] << shift-16)-1 ^ 255;
        subnet_datas->size[0] = 255;
    }
    else{
        subnet_datas->size[3] = 0;
        subnet_datas->size[2] = 0;
        subnet_datas->size[1] = 0;
        subnet_datas->size[0] = (full_mask[0] << shift-24)-1 ^ 255;
    }
}

void short_mask_to_dotted_mask(struct Subnet_data *subnet_datas, int mask){
    unsigned int shift = 32 - mask;
    unsigned char full_mask[4] = {1, 1, 1, 1};

    if (shift < 8){
        subnet_datas->mask[3] = ((full_mask[3] << shift)-1) ^ 255;
        subnet_datas->mask[2] = 255;
        subnet_datas->mask[1] = 255;
        subnet_datas->mask[0] = 255;
    }
    else if (shift < 16){
        subnet_datas->mask[3] = 0;
        subnet_datas->mask[2] = (full_mask[2] << shift-8)-1 ^ 255;
        subnet_datas->mask[1] = 255;
        subnet_datas->mask[0] = 255;
    }
    else if (shift < 24){
        subnet_datas->mask[3] = 0;
        subnet_datas->mask[2] = 0;
        subnet_datas->mask[1] = (full_mask[1] << shift-16)-1 ^ 255;
        subnet_datas->mask[0] = 255;
    }
    else{
        subnet_datas->mask[3] = 0;
        subnet_datas->mask[2] = 0;
        subnet_datas->mask[1] = 0;
        subnet_datas->mask[0] = (full_mask[0] << shift-24)-1 ^ 255;
    }
}

int check_big_mask(unsigned char *mask_array, struct Subnet_data *subnet_datas, int type){
    unsigned int mask = mask_array[0] | mask_array[1] << 8 | mask_array[2] << 16 | mask_array[3] << 24;
    unsigned int tmp_mask = mask;

    int cidr = 0;
    while (tmp_mask != 0){
        tmp_mask = tmp_mask & (tmp_mask-1);
        cidr += 1;
    }

    if (cidr <= 0 || cidr > 32){
        return -4;
    }
    if (type == 1) subnet_datas->cidr = cidr;
    else subnet_datas->second_cidr = cidr;
    struct Subnet_data tmp;
    short_mask_to_dotted_mask(&tmp, cidr);
    unsigned int correct_mask = tmp.mask[0] | tmp.mask[1] << 8 | tmp.mask[2] << 16 | tmp.mask[3] << 24;
    
    if (correct_mask == mask){
        return 0;
    }
    return -4;
}

int verify_ip_mask(char *ip_mask, struct Subnet_data *subnet_datas, int type){
    char tmp[100];
    strcpy(tmp, ip_mask);
    regex_t reegex;
    int is_short_mask = regcomp( &reegex, "^/", REG_EXTENDED);
    is_short_mask = regexec( &reegex, tmp, 0, NULL, 0);
    int flag = 0;

    if (is_short_mask == 0){
        int mask = atoi(strtok(tmp, "/"));
        if (mask < 1 || mask > 32){
            return -3;
        }
        if (type == 2){
            short_mask_to_dotted_mask2(subnet_datas, mask);
            subnet_datas->second_cidr = mask;
        }
        else {
            short_mask_to_dotted_mask(subnet_datas, mask);
            subnet_datas->cidr = mask;
        }
        return flag;
    }

    int i = 0;
    char *token = strtok(tmp, ".");
    while (token != NULL) {
        int tmp = atoi(token);
        if (type == 0) subnet_datas->ip[i] = tmp;
        else if (type == 1) subnet_datas->mask[i] = tmp;
        else subnet_datas->size[i] = tmp;
        if (tmp > 255){
            flag = -2;
        }
        i++;
        token = strtok(NULL, ".");
    }

    if (type == 1 && flag == 0){
        flag = check_big_mask(subnet_datas->mask, subnet_datas, 1);
    }

    if (type == 2 && flag == 0){
        flag = check_big_mask(subnet_datas->size, subnet_datas, 2);
    }

    return flag;
}

void get_ip_mask(struct Subnet_data *subnet_datas, char *user_string, char *directive_token){
    char tmp[1024];
    strcpy(tmp, user_string);
    char* token = strtok(tmp, directive_token);


    char ip_address[50];
    strcpy(ip_address, token);

    token = strtok(NULL, " MASK ");
    char mask[50];
    strcpy(mask, token);

    char number[10];
    char size[50];

    if (strcmp(directive_token, RANDOM_SUBNETS) == 0){
        token = strtok(NULL, " NUMBER ");
        strcpy(number, token);
        token = strtok(NULL, " SIZE ");
        strcpy(size, token);
    }


    int flag = 0;
    flag = verify_ip_mask(ip_address, subnet_datas, 0);
    if (flag != 0){
        subnet_datas->operation = flag;
        return;
    }

    flag = verify_ip_mask(mask, subnet_datas, 1);
    if (flag != 0){
        subnet_datas->operation = flag;
        return;
    }

    if (strcmp(directive_token, RANDOM_SUBNETS) == 0){
        if (atoi(number) > MAX_SUBNET){
            subnet_datas->operation = -5;
            return;
        }
        subnet_datas->number = atoi(number);

        flag = verify_ip_mask(size, subnet_datas, 2);
    }

    
    subnet_datas->operation = flag;
}

void print_ip_data(struct Subnet_data *subnet_datas){
    printf("IP ADDRESS: ");
    for (int i=0; i<4; i++){
        printf("%d", subnet_datas->ip[i]);
        if (i != 3){
            printf(".");
        }
    }
    printf("\nMASK: ");
    for (int i=0; i<4; i++){
        printf("%d", subnet_datas->mask[i]);
        if (i != 3){
            printf(".");
        }
    }
    printf("\nOPERATION: %d\n", subnet_datas->operation);
}

void verify_operation(char *user_string, struct Subnet_data *subnet_datas){
    regex_t reegex;
    subnet_datas->operation = -1;

    //Verifying if BROADCAST IP
    int broadcast_ip;
    char regex[200] = "^";
    strcat(regex, BROADCAST_IP);
    strcat(regex, REGEX_IP_MASK);
    strcat(regex, "\r");
    strcat(regex, "\n");

    broadcast_ip = regcomp( &reegex, regex, REG_EXTENDED);
    broadcast_ip = regexec( &reegex, user_string, 0, NULL, 0);

    if (broadcast_ip == 0){
        printf("\nBROADCAST\n");
        get_ip_mask(subnet_datas, user_string, BROADCAST_IP);
        if (subnet_datas->operation == 0){
            subnet_datas->operation = 0;
        }
        return;
    }

    //Verifying if NETWORK NUMBER
    int network_number;
    char regex2[200] = "^";
    strcat(regex2, NETWORK_NUMBER);
    strcat(regex2, REGEX_IP_MASK);
    strcat(regex2, "\r");
    strcat(regex2, "\n");

    network_number = regcomp( &reegex, regex2, REG_EXTENDED);
    network_number = regexec( &reegex, user_string, 0, NULL, 0);

    if (network_number == 0){
        printf("\nNETWORK NUMBER\n");
        get_ip_mask(subnet_datas, user_string, NETWORK_NUMBER);
        if (subnet_datas->operation == 0){
            subnet_datas->operation = 1;
        }
        return;
    }

    //Verifying if HOSTS RANGE
    int hosts_range;
    char regex3[200] = "^";
    strcat(regex3, HOSTS_RANGE);
    strcat(regex3, REGEX_IP_MASK);
    strcat(regex3, "\r");
    strcat(regex3, "\n");

    hosts_range = regcomp( &reegex, regex3, REG_EXTENDED);
    hosts_range = regexec( &reegex, user_string, 0, NULL, 0);

    if (hosts_range == 0){
        printf("\nHOSTS RANGE\n");
        get_ip_mask(subnet_datas, user_string, HOSTS_RANGE);
        if (subnet_datas->operation == 0){
            subnet_datas->operation = 2;
        }
        return;
    }

    //Verifying if RANDOM SUBNET
    int random_subnet;
    char regex4[400] = "^";
    strcat(regex4, RANDOM_SUBNETS);
    strcat(regex4, REGEX_IP_MASK);
    strcat(regex4, RANDOM_SUBNETS_REGEX);
    strcat(regex4, "\r");
    strcat(regex4, "\n");

    hosts_range = regcomp( &reegex, regex4, REG_EXTENDED);
    hosts_range = regexec( &reegex, user_string, 0, NULL, 0);

    if (hosts_range == 0){
        printf("\nRANDOM SUBNET\n");
        get_ip_mask(subnet_datas, user_string, RANDOM_SUBNETS);
        if (subnet_datas->operation == 0){
            subnet_datas->operation = 3;
        }
        return;
    }
}

char *array_to_dotted_ip(unsigned char *ip, int flag){
    char network_number[20] = "";
    char number_str[5];

    for (int i=0; i<4; i++){
        sprintf(number_str, "%d", ip[i]);
        strcat(network_number, number_str);
        if (i < 3) strcat(network_number, ".");
        // else strcat(network_number, "\n");
    }
    if (flag == 0){
        strcat(network_number, "\n");
    }
    char *tmp = network_number;
    return tmp;
}

char *build_hosts_range(unsigned char *ip1, unsigned char *ip2){
    char network_number[40] = "";
    char number_str[5];

    for (int i=0; i<4; i++){
        sprintf(number_str, "%d", ip1[i]);
        strcat(network_number, number_str);
        if (i < 3) strcat(network_number, ".");
        else strcat(network_number, " - ");
    }

    for (int i=0; i<4; i++){
        sprintf(number_str, "%d", ip2[i]);
        strcat(network_number, number_str);
        if (i < 3) strcat(network_number, ".");
        else strcat(network_number, "\n");
    }

    char *tmp = network_number;
    return tmp;
}

int is_subnet_in_list(unsigned int random_subnets[], unsigned int subnet, int length){
    for (int i=0; i<length; i++){
        if (subnet == random_subnets[i]){
            return 0;
        }
    }
    return 1;
}

void get_network_number(struct Subnet_data *subnet_datas){
    for (int i=0; i<4; i++){
        subnet_datas->network_address[i] = subnet_datas->ip[i] & subnet_datas->mask[i];
    }
}

void get_broadcast_ip(struct Subnet_data *subnet_datas){
    get_network_number(subnet_datas);
    for (int i=0; i<4; i++){
        subnet_datas->broadcast_ip[i] = subnet_datas->network_address[i] + (~subnet_datas->mask[i]);
    }
}

void get_hosts_range(struct Subnet_data *subnet_datas){
    get_network_number(subnet_datas);
    get_broadcast_ip(subnet_datas);
    for (int i=0; i<4; i++){
        subnet_datas->starting_address[i] = subnet_datas->network_address[i];
        subnet_datas->last_address[i] = subnet_datas->broadcast_ip[i];
    }
    subnet_datas->starting_address[3] += 1;
    subnet_datas->last_address[3] -= 1;
}

char *get_response(struct Subnet_data *subnet_datas){
    int operation = subnet_datas->operation;
    unsigned int random_subnets[subnet_datas->number];

    switch (operation){
        case 0:
            get_broadcast_ip(subnet_datas);
            return array_to_dotted_ip(subnet_datas->broadcast_ip, 0);
        
        case 1:
            get_network_number(subnet_datas);
            return array_to_dotted_ip(subnet_datas->network_address, 0);

        case 2:
            get_hosts_range(subnet_datas);
            if (subnet_datas->cidr > 30){
                return "There isn't a usable host range for cidr masks greater than 30\n";
            }
            return build_hosts_range(subnet_datas->starting_address, subnet_datas->last_address);
        
        case 3:
            get_hosts_range(subnet_datas);
            if (subnet_datas->second_cidr <= subnet_datas->cidr){
                return "Second CIDR mask must be greater than first CIDR mask\n";
            }
            double total_subnets = pow(2, 32-subnet_datas->cidr)/pow(2, 32-subnet_datas->second_cidr);
            if (total_subnets < subnet_datas->number){
                return "You requested an amount of random subnets that is greater than the possibles for the IP and MASK given\n";
            }

            unsigned int lower = subnet_datas->starting_address[3] | subnet_datas->starting_address[2] << 8 | subnet_datas->starting_address[1] << 16 | subnet_datas->starting_address[0] << 24;
            unsigned int upper = (subnet_datas->last_address[3]) | (subnet_datas->last_address[2] << 8) | (subnet_datas->last_address[1] << 16) | (subnet_datas->last_address[0] << 24);
            unsigned int second_mask = (subnet_datas->size[3]) | (subnet_datas->size[2] << 8) | (subnet_datas->size[1] << 16) | (subnet_datas->size[0] << 24);

            srand(time(NULL));
            unsigned int subnet;
            for (int i=0; i<subnet_datas->number; i++){
                subnet = ((rand() % (upper - lower + 1)) + lower) & second_mask;
                while (is_subnet_in_list(random_subnets, subnet, i) == 0){
                    subnet = ((rand() % (upper - lower + 1)) + lower) & second_mask;
                }
                random_subnets[i] = subnet;
            }

            unsigned char bytes[4];
            char random_subnets_str[10000000] = "";
            int count = 0;
            char number_str[10];

            while (count < subnet_datas->number){
                sprintf(number_str, "%d", count+1);
                strcat(random_subnets_str, number_str);
                strcat(random_subnets_str, ". ");
                bytes[3] = random_subnets[count];
                bytes[2] = random_subnets[count] >> 8;
                bytes[1] = random_subnets[count] >> 16;
                bytes[0] = random_subnets[count] >> 24;
                strcat(random_subnets_str, array_to_dotted_ip(bytes, 1));
                strcat(random_subnets_str, "/");
                sprintf(number_str, "%d", subnet_datas->second_cidr);
                strcat(random_subnets_str, number_str);
                strcat(random_subnets_str, "\n");
                count++;
            }

            char *tmp = random_subnets_str;
            return tmp;

        case -1:
            return "You either entered a wrong directive or didn't entered a positive number with 1, 2 or 3 digits.\n";
        
        case -2:
            return "Numbers in ip address and mask must be greater than -1 and lower than 256.\n";

        case -3:
            return "CIDR masks must be greater than 0 and lower than 33.\n";
        
        case -4:
            return "You entered an invalid mask. Make sure to use CIDR value between 1 and 32\n";

        case -5:
            return "The MAX SUBNET number possible for calculating random subnet is 4194304\n";

        default:
            return "Something went wrong.\n";
    }
}

char *subnet_calculator(char *user_input){
    struct Subnet_data subnet_datas;
    verify_operation(user_input, &subnet_datas);
    // print_ip_data(&subnet_datas);
    return get_response(&subnet_datas);
}

// int main(){
//     char *test = "GET RANDOM SUBNETS NETWORK NUMBER 10.0.0.0 MASK /8 NUMBER 3 SIZE /24\r\n";
//     // char *test = "GET BROADCAST IP 198.168.0.1 MASK 255.255.0.0\r\n";
//     char *response = subnet_calculator(test);
//     printf("Response: %s\n", response);

// }

// // // //PRUEBAS
// int main(){
//     struct Subnet_data subnet_datas;
//     char *prueba;
//     char *response;
    
//     prueba = "GET BROADCAST IP 198.168.0.1 MASK 255.255.0.0\r\n";
//     response = subnet_calculator(prueba);
//     printf("Broadcast ip: %s; Esperado: 192.168.255.255\n", response);

//     prueba = "GET NETWORK NUMBER IP 198.168.0.1 MASK 255.255.0.0\r\n";
//     response = subnet_calculator(prueba);
//     printf("Network number: %s; Esperado: 192.168.0.0\n", response);

//     prueba = "GET HOSTS RANGE IP 198.168.0.1 MASK 255.255.0.0\r\n";
//     response = subnet_calculator(prueba);
//     printf("Hosts range: %s; Esperado: 	192.168.0.1 - 192.168.255.254\n", response);
    

//     prueba = "GET BROADCAST IP 172.16.0.56 MASK /25\r\n";
//     response = subnet_calculator(prueba);
//     printf("Broadcast ip: %s; Esperado: 172.16.0.127\n", response);

//     prueba = "GET NETWORK NUMBER IP 172.16.0.56 MASK /25\r\n";
//     response = subnet_calculator(prueba);
//     printf("Network number: %s; Esperado: 172.16.0.0\n", response);
    
//     prueba = "GET HOSTS RANGE IP 172.16.0.56 MASK /25\r\n";
//     response = subnet_calculator(prueba);
//     printf("Hosts range: %s; Esperado: 	172.16.0.1 - 172.16.0.126\n", response);


//     prueba = "GET BROADCAST IP 10.0.8.5 MASK /12\r\n";
//     response = subnet_calculator(prueba);
//     printf("Broadcast ip: %s; Esperado: 10.15.255.255\n", response);
    
//     prueba = "GET NETWORK NUMBER IP 10.0.8.5 MASK /12\r\n";
//     response = subnet_calculator(prueba);
//     printf("Network number: %s; Esperado: 10.0.0.0\n", response);

//     prueba = "GET HOSTS RANGE IP 10.0.8.5 MASK /12\r\n";
//     response = subnet_calculator(prueba);
//     printf("Hosts range: %s; Esperado: 	10.0.0.1 - 10.15.255.254\n", response);


//     prueba = "GET BROADCAST IP 10.8.2.5 MASK /29\r\n";
//     response = subnet_calculator(prueba);
//     printf("Broadcast ip: %s; Esperado: 10.8.2.7\n", response);
    
//     prueba = "GET NETWORK NUMBER IP 10.8.2.5 MASK /29\r\n";
//     response = subnet_calculator(prueba);
//     printf("Network number: %s; Esperado: 10.8.2.0\n", response);

//     prueba = "GET HOSTS RANGE IP 10.8.2.5 MASK /29\r\n";
//     response = subnet_calculator(prueba);
//     printf("Hosts range: %s; Esperado: 	10.8.2.1 - 10.8.2.6\n", response);


//     prueba = "GET BROADCAST IP 201.203.117.214 MASK /5\r\n";
//     response = subnet_calculator(prueba);
//     printf("Broadcast ip: %s; Esperado: 207.255.255.255\n", response);
    
//     prueba = "GET NETWORK NUMBER IP 201.203.117.214 MASK /5\r\n";
//     response = subnet_calculator(prueba);
//     printf("Network number: %s; Esperado: 200.0.0.0\n", response);

//     prueba = "GET HOSTS RANGE IP 201.203.117.214 MASK /5\r\n";
//     response = subnet_calculator(prueba);
//     printf("Hosts range: %s; Esperado: 200.0.0.1 - 207.255.255.254\n", response);
    
//     return 0;
// }

