enum request_command_type{SSH_MSG_CHANNEL_REQUEST=98,SSH_MSG_CHANNEL_SUCCESS=99,SSH_MSG_CHANNEL_FAILURE=100};

struct serverssh{
    char type;
    char strings[6+1025];
};

struct serversshresponse{
    char type;
    char retour;
};