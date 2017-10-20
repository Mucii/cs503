#include <xinu.h>

devcall pipputc(struct dentry *devptr, char ch) {
    intmask mask;

    mask=disable();
    
    if(SYSERR==pipwrite(devptr, &ch, 1)){
        restore(mask);
        return SYSERR;
    }else{
        restore(mask);
        return 1;
    }

}


