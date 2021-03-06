# compileall.sh

#!/bin/bash
gcc --std=gnu99 -o enc_server enc_server.c -lm
gcc --std=gnu99 -o enc_client enc_client.c -lm
gcc --std=gnu99 -o dec_server dec_server.c -lm
gcc --std=gnu99 -o dec_client dec_client.c -lm
gcc --std=gnu99 -o keygen keygen.c -lm