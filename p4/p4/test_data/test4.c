/* Simple counter increment
*/

#include "p4.h"

int main()
{
  counter_t c;
  Counter_Init(&c, 0);
  Counter_Increment(&c);

  int val = Counter_GetValue(&c);
  if (val == 1)
      return 0;
  else
      return -1;
}

