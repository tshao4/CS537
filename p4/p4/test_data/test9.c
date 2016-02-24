/* Simple hash insert
*/

#include "p4.h"

int main()
{
    int element = 255;
    unsigned int key = 127;
    hash_t hs;
    Hash_Init(&hs, 32);
    Hash_Insert(&hs, (void*)&element, key);
    void *val = Hash_Lookup(&hs, key);
    if (val)
	return 0;
    else
	return -1;
}
