
struct Subnet_data {
   unsigned char  ip[4]; //bit array storing ip address
   unsigned char  mask[4]; //bit array storing mask
   unsigned char  network_address[4];
   unsigned char  broadcast_ip[4];
   unsigned char  starting_address[4];
   unsigned char  last_address[4];
   unsigned char size[4];
   int number;
   int cidr;
   int second_cidr;
   int   operation; //stores the operation to do or an code error
};

void verify_operation(char *user_string, struct Subnet_data *);
/*
-----------------------------------------------------------------------
    verify_operation
    Input: string written by the client
    Output: void
    Functioning: verify if the string meets the directives, and 
        updates the struct pointer with the ip and mask and operation
-----------------------------------------------------------------------
*/

void get_ip_mask(struct Subnet_data *, char *user_string, char *directive_token);
/*
-----------------------------------------------------------------------
    get_ip_mask
    Input: pointer to struct, string written by client, string with the 
        operation to do
    Output: void
    Functioning: tokenize the input and get the ip address (xxx.xxx.xxx.xxx) 
    and mask (xxx.xxx.xxx.xxx) or (/xx), then updates the struct operation
    data field with the operation or code error.
-----------------------------------------------------------------------
*/

void print_ip_data(struct Subnet_data *);
/*
-----------------------------------------------------------------------
    print_ip_data
    Input: pointer to struct
    Output: void
    Functioning: print the information stored in a Subnet_data struct
-----------------------------------------------------------------------
*/

int verify_ip_mask(char *ip_mask, struct Subnet_data *, int type);
/*
-----------------------------------------------------------------------
    verify_ip_mask
    Input: string with ip or mask with format (xxx.xxx.xxx.xxx) or (/xx), 
        pointer to struct, int indicating if ip_mask is ip or mask
    Output: integer, 0 if data is correct, -1 if not.
    Functioning: verify if all the octects are lower than 256, and if
        mask is short, checks if is greater than 7 and lower than 33
-----------------------------------------------------------------------
*/

void short_mask_to_dotted_mask(struct Subnet_data *, int mask);
/*
-----------------------------------------------------------------------
    short_mask_to_dotted_mask
    Input: pointer to struct, integer indicating the mask (between 1 and 32)
    Output: void
    Functioning: transforms a short mask to a dotted mask (x.x.x.x), updates
        the mask value in the struct
-----------------------------------------------------------------------
*/

char *array_to_dotted_ip(unsigned char *ip, int flag);
/*
-----------------------------------------------------------------------
    array_to_dotted_ip
    Input: array of unsigned int, it contains the ip
    Output: string with the ip separated by dots
    Functioning: transforms an unsigned int array to a string,
        separated by dots
-----------------------------------------------------------------------
*/

char *build_hosts_range(unsigned char *ip1, unsigned char *ip2);
/*
-----------------------------------------------------------------------
    build_hosts_range
    Input: two array of unsigned int containing initial and final
        address in the hosts range
    Output: string with hosts range
    Functioning: transforms two unsigned int array to a string,
        separated by dots
-----------------------------------------------------------------------
*/

void get_broadcast_ip(struct Subnet_data *);
/*
-----------------------------------------------------------------------
    get_broadcast_ip
    Input: pointer to struct that contains a ip and a mask
    Output: pointer to char with the broadcast ip
    Functioning: calculates the broadcast ip with the values stored in
        the struct
-----------------------------------------------------------------------
*/

void get_network_number(struct Subnet_data *);
/*
-----------------------------------------------------------------------
    get_network_number
    Input: pointer to struct that contains a ip and a mask
    Output: pointer to char with the broadcast ip
    Functioning: calculates the broadcast ip with the values stored in
        the struct
-----------------------------------------------------------------------
*/

void get_hosts_range(struct Subnet_data *);
/*
-----------------------------------------------------------------------
    get_hosts_range
    Input: pointer to struct that contains a ip and a mask
    Output: pointer to char with the broadcast ip
    Functioning: calculates the broadcast ip with the values stored in
        the struct
-----------------------------------------------------------------------
*/

char *get_response(struct Subnet_data *);
/*
-----------------------------------------------------------------------
    get_response
    Input: pointer to struct
    Output: string with the response of the user input
    Functioning: perform the operation depending on the input 
        of the user, then return the response
-----------------------------------------------------------------------
*/

char *subnet_calculator(char *user_input);
/*
-----------------------------------------------------------------------
    subnet_calculator
    Input: string with the string the user wrote
    Output: string with the response of the user input
    Functioning: manages the subnet-calculator system, this function
        is called from the echo_server
-----------------------------------------------------------------------
*/

int check_big_mask(unsigned char *mask_array, struct Subnet_data *, int type);
/*
-----------------------------------------------------------------------
    check_big_mask
    Input: array of unsigned char containing a mask
    Output: integer, 0 if correct, -4 if not
    Functioning: check if a big mask introduced by the user
        is valid
-----------------------------------------------------------------------
*/

int is_subnet_in_list(unsigned int random_subnets[], unsigned int subnet, int lenght);