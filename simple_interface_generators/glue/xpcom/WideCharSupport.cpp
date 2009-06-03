#include "WideCharSupport.h"

void
ConvertWcharToAString
(
 const wchar_t* aSource,
 nsAString& aDest
)
{
#ifdef WCHAR_T_IS_16BIT
  aDest = (PRUnichar*)aSource;
#else
  PRUnichar buf[1024];
  int i;
  buf[1023] = 0;
  aDest.Truncate();
  while (aSource != NULL)
  {
    for (i = 0; i < 1023; i++)
    {
      if (*aSource == 0)
      {
        aSource = NULL;
        buf[i] = 0;
        break;
      }
      else
      {
#ifdef WCHAR_T_IS_32BIT
        buf[i] = (PRUnichar)(uint32_t)*aSource++;
#else
        buf[i] = (PRUnichar)(uint64_t)*aSource++;
#endif
      }
    }
    aDest.Append(buf);
  }
#endif
}

void
ConvertAStringToWchar
(
 const nsAString& aSource,
 wchar_t** aDest
)
{
#ifdef WCHAR_T_IS_16BIT
  *aDest = CDA_wcsdup((const wchar_t*)aSource.BeginReading());
#else
  int i = aSource.Length();
  *aDest = (wchar_t*)malloc(sizeof(wchar_t) * (i + 1));
  (*aDest)[i] = 0;
  while (i)
  {
    i--;
    (*aDest)[i] = (wchar_t)(uint32_t)aSource[i];
  }
#endif
}
