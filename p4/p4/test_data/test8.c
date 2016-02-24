/* list: delete non-existing element
*/

#include "p4.h"

int main()
{
    // int element = 255;
    unsigned int key = 127;
    list_t lst;
    List_Init(&lst);
    List_Delete(&lst, key);
    void *val = List_Lookup(&lst, key);
    if (!val)
	return 0;
    else
	return -1;
}

