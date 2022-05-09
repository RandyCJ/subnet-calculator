#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

#define BROADCAST_IP "GET BROADCAST IP "
#define NETWORK_NUMBER "GET NETWORK NUMBER IP "
#define HOSTS_RANGE "GET HOSTS RANGE IP "
#define RANDOM_SUBNETS "GET RANDOM SUBNETS NETWORK NUMBER "
#define REGEX_IP_MASK "[0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9] \
MASK ([0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9][.][0-9]?[0-9]?[0-9]|/[0-9]?[0-9])"

struct Subnet_data {
   char  ip[50];
   char  mask[50];
   int   operation;
};

int verify_ip_mask(char *ip_mask){
    char tmp[20];
    strcpy(tmp, ip_mask);
    regex_t reegex;
    int is_short_mask = regcomp( &reegex, "^/", REG_EXTENDED);
    is_short_mask = regexec( &reegex, tmp, 0, NULL, 0);

    if (is_short_mask == 0){
        int mask = atoi(strtok(tmp, "/"));
        if (mask < 8 || mask > 32){
            printf("Short mask must be between 8-32; Error /%d\n", mask);
            return -1;
        }
    }

    char *token = strtok(tmp, ".");
    while (token != NULL) {
        if (atoi(token) > 255){
            printf("Number must be between 0-255; Error %s\n", token);
            return -1;
        }
        token = strtok(NULL, ".");
    }
    return 0;
}

struct Subnet_data get_ip_mask(char *user_string, char *directive_token){
    struct Subnet_data subnet_datas;
    char tmp[16];
    strcpy(tmp, user_string);
    char* token = strtok(tmp, directive_token);

    char ip_address[32];
    strcpy(ip_address, token);

    token = strtok(NULL, " MASK ");
    char mask[32];
    strcpy(mask, token);

    if (verify_ip_mask(ip_address) < 0){
        subnet_datas.operation = -1;
    }
    if (verify_ip_mask(mask) < 0){
        subnet_datas.operation = -1;
    }

    strcpy(subnet_datas.ip, ip_address);
    strcpy(subnet_datas.mask, mask);

    return subnet_datas;
}

void print_ip_data(struct Subnet_data subnet_datas){
    printf("\nIP ADDRESS: %s\n", subnet_datas.ip);
    printf("MASK: %s\n", subnet_datas.mask);
    printf("OPERATION: %d\n", subnet_datas.operation);
}

struct Subnet_data verify_operation(char *user_string){
    regex_t reegex;
    struct Subnet_data subnet_datas;

    //Verifying if BROADCAST IP
    int broadcast_ip;
    char regex[200] = "^";
    strcat(regex, BROADCAST_IP);
    strcat(regex, REGEX_IP_MASK);
    strcat(regex, "$");

    broadcast_ip = regcomp( &reegex, regex, REG_EXTENDED);
    broadcast_ip = regexec( &reegex, user_string, 0, NULL, 0);

    if (broadcast_ip == 0){
        subnet_datas = get_ip_mask(user_string, BROADCAST_IP);
        if (subnet_datas.operation != -1){
            subnet_datas.operation = 0;
        }
        return subnet_datas;
    }

    //Verifying if NETWORK NUMBER
    int network_number;
    char regex2[200] = "^";
    strcat(regex2, NETWORK_NUMBER);
    strcat(regex2, REGEX_IP_MASK);
    strcat(regex2, "$");

    network_number = regcomp( &reegex, regex2, REG_EXTENDED);
    network_number = regexec( &reegex, user_string, 0, NULL, 0);

    if (network_number == 0){
        subnet_datas = get_ip_mask(user_string, NETWORK_NUMBER);
        if (subnet_datas.operation != -1){
            subnet_datas.operation = 1;
        }
        return subnet_datas;
    }

    //Verifying if HOSTS RANGE
    int hosts_range;
    char regex3[200] = "^";
    strcat(regex3, HOSTS_RANGE);
    strcat(regex3, REGEX_IP_MASK);
    strcat(regex3, "$");

    hosts_range = regcomp( &reegex, regex3, REG_EXTENDED);
    hosts_range = regexec( &reegex, user_string, 0, NULL, 0);

    if (hosts_range == 0){
        subnet_datas = get_ip_mask(user_string, HOSTS_RANGE);
        if (subnet_datas.operation != -1){
            subnet_datas.operation = 2;
        }
        return subnet_datas;
    }

    return subnet_datas;

    int network_number_ip;
    int hosts_range_ip;
    int rand_subnets_net_num;

}

int main(){

    char *prueba = "GET BROADCAST IP 14.332.4.243 MASK 25.512.5.128";
    //struct Subnet_data subnet_datas = verify_operation(prueba);
    print_ip_data(verify_operation(prueba));
    
    prueba = "GET NETWORK NUMBER IP 112.12.12.6 MASK 255.5.25.128";
    //verify_operation(prueba);
    print_ip_data(verify_operation(prueba));


    prueba = "GET HOSTS RANGE IP 214.126.9.6 MASK 255.5.25.328";
    //verify_operation(prueba);
    print_ip_data(verify_operation(prueba));

    prueba = "GET BROADCAST IP 14.332.4.243 MASK /8";
    //struct Subnet_data subnet_datas = verify_operation(prueba);
    print_ip_data(verify_operation(prueba));
    
    prueba = "GET NETWORK NUMBER IP 112.12.12.6 MASK /16";
    //verify_operation(prueba);
    print_ip_data(verify_operation(prueba));


    prueba = "GET HOSTS RANGE IP 214.126.9.6 MASK /32";
    //verify_operation(prueba);
    print_ip_data(verify_operation(prueba));


    prueba = "GET BROADCAST IP 1723.16.0.56 MASK 255.5.25.128";
    verify_operation(prueba);

    prueba = "GET NETWORK NUMBER IP 133.16.0.56 MASK /7";
    verify_operation(prueba);

    prueba = "GET HOSTS RANGE IP 173.16.0.56 MASK /33";
    verify_operation(prueba);

    return 0;
}


/*
Quedé en que tengo guardado la ip y la máscara en un struct, junto con un entero
que me dice que tipo de operación tengo que realizar, ya está validado el IP y la mascara.
Tareas:
    - Pasar ip y mascara a binario para empezar a realizar las operaciones bit-wise
*/