
pid_t spawnPid = fork();
switch (spawnPid) {
    case -1:
        perror("fork()\n");
        exit(1);
        break;
    case 0:
        // child process instructions

        exit(0);
        break;
    default:
        // keep track of new child
        currChildren[*numOfChildren] = spawnPid;
        // increment numOfChildren
        *numOfChildren = *numOfChildren + 1;

        // parent process instructions


        break;
}