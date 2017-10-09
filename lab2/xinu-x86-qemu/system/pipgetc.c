#include <xinu.h>

devcall pipgetc(struct dentry *devptr) {
    intmask mask;
    char ch;

    mask=disable();
    
    if(SYSERR==pipread(devptr, &ch, 1)){
        restore(mask);
        return SYSERR;
    }else{
        restore(mask);
        return ch;
    }
}

