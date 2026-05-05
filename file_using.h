#ifndef FILE_USING_H
#define FILE_USING_H

bool check_file_opening(const char* const input_address, FILE** file_ptr, const char* const access_rights);
bool check_file_closing(FILE* input_address);

#endif //FILE_USING_H
