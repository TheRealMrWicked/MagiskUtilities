char* os();

char* commandoutput(char command[]);

bool filexists(char filename[]);

void wildcardrename(char oldname[], char newname[]);

char* getextension(char filename[]);

char* removefileextension(char filename[]);

void removefolder(char path[]);

void copyfolder(char srcfolder[], char dstfolder[]);