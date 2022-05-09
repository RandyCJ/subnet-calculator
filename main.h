struct Subnet_data {
   unsigned char  ip[4]; //bit array storing ip address
   unsigned char  mask[4]; //bit array storing mask
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
char *get_broadcast_ip(struct Subnet_data *);
/*
-----------------------------------------------------------------------
    get_broadcast_ip
    Input: pointer to struct that contains a ip and a mask
    Output: pointer to char with the broadcast ip
    Functioning: calculates the broadcast ip with the values stored in
        the struct
-----------------------------------------------------------------------
*/

char *get_network_number(struct Subnet_data *);
/*
-----------------------------------------------------------------------
    get_network_number
    Input: pointer to struct that contains a ip and a mask
    Output: pointer to char with the broadcast ip
    Functioning: calculates the broadcast ip with the values stored in
        the struct
-----------------------------------------------------------------------
*/

char *get_hosts_range(struct Subnet_data *);
/*
-----------------------------------------------------------------------
    get_hosts_range
    Input: pointer to struct that contains a ip and a mask
    Output: pointer to char with the broadcast ip
    Functioning: calculates the broadcast ip with the values stored in
        the struct
-----------------------------------------------------------------------
*/