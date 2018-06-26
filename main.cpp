//
//  main.cpp
//  SysDig
//
//  Created by Ranjit Borra on 6/23/18.
//  Copyright © 2018 Ranjit Borra. All rights reserved.
//


#include <iostream>
#include "single_shared_channel.h"
#include "udp_server.h"

int main(int argc, const char * argv[]) {
    auto channel = sysdig::SingleSharedChannel::getSingleSharedChannel("127.0.1.1", 8200, 100, 100);
    char buff[] = "Hello SysDig";
    if(channel)
        channel->sendMessage(buff, 0);
    return 0;
}
