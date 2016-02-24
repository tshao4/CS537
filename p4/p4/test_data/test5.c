/* Simple counter decrement
*/

#include "p4.h"

int main()
{
  counter_t c;
  Counter_Init(&c, 1);
  Counter_Decrement(&c);

  int val = Counter_GetValue(&c);
  if (val == 0)
      return 0;
  else
      return -1;
}

