#pragma once

enum request_type{SSH_MSG_USERAUTH_REQUEST=50,SSH_MSG_USERAUTH_FAILURE=51,SSH_MSG_USERAUTH_SUCCESS=52};

struct ssh{
    char user_request;
    char strings[65+4+9+129];
};