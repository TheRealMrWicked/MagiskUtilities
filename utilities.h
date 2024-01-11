char* os();

// Command Manipulation

char* commandOutput(char *command);

int supressOutput (char *command);

// File Manipulation

bool fileExists(char *fileName);

void wildcardRename(char *oldName, char *newName);

char* getExtension(char *fileName);

char* removeFileExtension(char *fileName);

void copyFile(char *sourceFile, char *destinationFile);

// Folder Manipulation

void removeFolder(char *folder);

void copyFolder(char *sourceFolder, char *destinationFolder);