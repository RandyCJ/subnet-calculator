#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <math.h>
#include <time.h>
#include "subnet_calculator.h"

#define BROADCAST_IP "GET BROADCAST IP "
#define NETWORK_NUMBER "GET NETWORK NUMBER IP "
#define HOSTS_RANGE "GET HOSTS RANGE IP "
#define RANDOM_SUBNETS "GET RANDOM SUBNETS NETWORK NUMBER "
#define REGEX_IP_MASK "[0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9] \
MASK ([0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9]|/[0-9]?[0-9])"
#define RANDOM_SUBNETS_REGEX " NUMBER [0-9]?[0-9]?[0-9]?[0-9]?[0-9]?[0-9]?[0-9]?[0-9]?[0-9]?[0-9] SIZE ([0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9]|/[0-9]?[0-9])"
#define MAX_SUBNET 4194304

unsigned int cidr_to_mask(unsigned int cidrValue){ 
    /*Function taken from Brandon James at https://neverthenetwork.com/notes/bitwise_subnetting*/
    // left shift 1 by 32 - cidr, subtract 1 from the result and XORing
    // it with a mask that has all bits set, yeilds the subnet mask
    return -1 ^ ((1 << (32 - cidrValue)) - 1);
}

int check_big_mask(struct Subnet_data *subnet_datas, int type){
    unsigned int mask;
    if (type == 1) mask = subnet_datas->mask;
    else mask = subnet_datas->size;
    unsigned int tmp_mask = mask;

    int cidr = 0;
    while (tmp_mask != 0){
        tmp_mask = tmp_mask & (tmp_mask-1);
        cidr += 1;
    }

    if (cidr < 8 || cidr > 32){
        return -4;
    }
    if (type == 1) subnet_datas->cidr = cidr;
    else subnet_datas->second_cidr = cidr;

    unsigned int correct_mask = cidr_to_mask(cidr);
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
        if (mask < 8 || mask > 32){
            return -3;
        }
        if (type == 2){
            subnet_datas->second_cidr = mask;
            subnet_datas->size = cidr_to_mask(mask);
        }
        else {
            subnet_datas->cidr = mask;
            subnet_datas->mask = cidr_to_mask(mask);
        }
        return flag;
    }

    int i = 0;
    char *token = strtok(tmp, ".");
    unsigned char tmp_array[4];
    while (token != NULL) {
        int tmp = atoi(token);
        tmp_array[i] = tmp;
        if (tmp > 255){
            flag = -2;
        }
        i++;
        token = strtok(NULL, ".");
    }

    if (type == 0) subnet_datas->ip = tmp_array[3] | tmp_array[2] << 8 | tmp_array[1] << 16 | tmp_array[0] << 24;
    else if (type == 1) subnet_datas->mask = tmp_array[3] | tmp_array[2] << 8 | tmp_array[1] << 16 | tmp_array[0] << 24;
    else subnet_datas->size = tmp_array[3] | tmp_array[2] << 8 | tmp_array[1] << 16 | tmp_array[0] << 24;

    if (type == 1 && flag == 0){
        flag = check_big_mask(subnet_datas, 1);
    }

    if (type == 2 && flag == 0){
        flag = check_big_mask(subnet_datas, 2);
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
    int is_random_subnet = strcmp(directive_token, RANDOM_SUBNETS);
    if (is_random_subnet == 0){
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

    if (is_random_subnet == 0){
        int int_number = atoi(number);
        if (int_number > MAX_SUBNET){
            subnet_datas->operation = -5;
            return;
        }
        subnet_datas->number = int_number;

        flag = verify_ip_mask(size, subnet_datas, 2);
    }

    subnet_datas->operation = flag;
}

void print_ip_data(struct Subnet_data *subnet_datas){
    printf("IP ADDRESS: ");
    unsigned char ip[4];
    ip[3] = subnet_datas->ip;
    ip[2] = subnet_datas->ip >> 8;
    ip[1] = subnet_datas->ip >> 16;
    ip[0] = subnet_datas->ip >> 24;
    for (int i=0; i<4; i++){
        printf("%d", ip[i]);
        if (i != 3) printf(".");
    }

    unsigned char mask[4];
    mask[3] = subnet_datas->mask;
    mask[2] = subnet_datas->mask >> 8;
    mask[1] = subnet_datas->mask >> 16;
    mask[0] = subnet_datas->mask >> 24;
    printf("\nMASK: ");
    for (int i=0; i<4; i++){
        printf("%d", mask[i]);
        if (i != 3) printf(".");
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

char *array_to_dotted_ip(unsigned int ip, int flag){
    char network_number[20] = "";
    char number_str[10];
    
    unsigned char bytes[4];
    bytes[3] = ip;
    bytes[2] = ip >> 8;
    bytes[1] = ip >> 16;
    bytes[0] = ip >> 24;

    for (int i=0; i<4; i++){
        sprintf(number_str, "%d", bytes[i]);
        strcat(network_number, number_str);
        if (i < 3) strcat(network_number, ".");
    }
    if (flag == 0) strcat(network_number, "\n");
    char *tmp = network_number;
    return tmp;
}

char *build_hosts_range(unsigned int ip1, unsigned int ip2){
    char network_number[40] = "";
    char number_str[10];

    unsigned char bytes1[4];
    bytes1[3] = ip1;
    bytes1[2] = ip1 >> 8;
    bytes1[1] = ip1 >> 16;
    bytes1[0] = ip1 >> 24;

    unsigned char bytes2[4];
    bytes2[3] = ip2;
    bytes2[2] = ip2 >> 8;
    bytes2[1] = ip2 >> 16;
    bytes2[0] = ip2 >> 24;

    for (int i=0; i<4; i++){
        sprintf(number_str, "%d", bytes1[i]);
        strcat(network_number, number_str);
        if (i < 3) strcat(network_number, ".");
        else strcat(network_number, " - ");
    }

    for (int i=0; i<4; i++){
        sprintf(number_str, "%d", bytes2[i]);
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
    subnet_datas->network_address = subnet_datas->ip & subnet_datas->mask;
}

void get_broadcast_ip(struct Subnet_data *subnet_datas){
    get_network_number(subnet_datas);
    subnet_datas->broadcast_ip = subnet_datas->network_address + (~subnet_datas->mask);
}

void get_hosts_range(struct Subnet_data *subnet_datas){
    get_network_number(subnet_datas);
    get_broadcast_ip(subnet_datas);
    subnet_datas->starting_address = subnet_datas->network_address+1;
    subnet_datas->last_address = subnet_datas->broadcast_ip-1;
}

char *get_random_subnets(struct Subnet_data *subnet_datas){
    int amount_of_randoms = subnet_datas->number;
    int random_subnets_array[amount_of_randoms+1];
    unsigned int lower = subnet_datas->starting_address;
    unsigned int upper = subnet_datas->last_address;
    unsigned int second_mask = subnet_datas->size;
    
    unsigned int subnet;
    char random_subnets_str[10000] = "";
    char number_str[10];
    for (int i=0; i<amount_of_randoms; i++){
        do{
            subnet = ((rand() % (upper - lower + 1)) + lower) & second_mask;
        }while (is_subnet_in_list(random_subnets_array, subnet, i) == 0);
        random_subnets_array[i] = subnet;
        sprintf(number_str, "%d", i+1);
        strcat(random_subnets_str, number_str);
        strcat(random_subnets_str, ". ");
        strcat(random_subnets_str, array_to_dotted_ip(subnet, 1));
        strcat(random_subnets_str, "/");
        sprintf(number_str, "%d", subnet_datas->second_cidr);
        strcat(random_subnets_str, number_str);
        strcat(random_subnets_str, "\n");
    }

    char *tmp = random_subnets_str;
    return tmp;
}

char *get_response(struct Subnet_data *subnet_datas){
    int operation = subnet_datas->operation;

    if (operation == 0){
        get_broadcast_ip(subnet_datas);
        return array_to_dotted_ip(subnet_datas->broadcast_ip, 0);
    }
    if (operation == 1){
        get_network_number(subnet_datas);
        return array_to_dotted_ip(subnet_datas->network_address, 0);
    }
    if (operation == 2){
        get_hosts_range(subnet_datas);
        if (subnet_datas->cidr > 30){
            return "There isn't a usable host range for cidr masks greater than 30\n";
        }
        return build_hosts_range(subnet_datas->starting_address, subnet_datas->last_address);
    }
    if (operation == 3){
        get_hosts_range(subnet_datas);
        if (subnet_datas->second_cidr <= subnet_datas->cidr){
            return "Second CIDR mask must be greater than first CIDR mask\n";
        }
        double total_subnets = pow(2, 32-subnet_datas->cidr)/pow(2, 32-subnet_datas->second_cidr);
        if (total_subnets < subnet_datas->number){
            return "You requested an amount of random subnets that is greater than the possibles for the IP and MASK given\n";
        }
        char *tmp = get_random_subnets(subnet_datas);
        return tmp;
    }
    if (operation == -1){
        return "You either entered a wrong directive or didn't entered a positive number with 1, 2 or 3 digits.\n";
    }
    if (operation == -2){
        return "Numbers in ip address and mask must be greater than -1 and lower than 256.\n";
    }
    if (operation == -3){
        return "CIDR masks must be greater than 7 and lower than 33.\n";
    }
    if (operation == -4){
        return "You entered an invalid mask. Make sure to use CIDR value between 1 and 32\n";
    }
    if (operation == -5){
        return "The MAX SUBNET number possible for calculating random subnet is 4194304\n";
    }
    else return "Something went wrong.\n";
    
}

char *subnet_calculator(char *user_input){
    srand(time(NULL));
    struct Subnet_data subnet_datas;
    verify_operation(user_input, &subnet_datas);
    return get_response(&subnet_datas);
}

