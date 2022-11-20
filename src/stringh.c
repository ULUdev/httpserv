#include "stringh.h"
#include <string.h>
#include <wctype.h>

int httpserv_streq(const char *s1, const char *s2) {
  if (strlen(s1) == strlen(s2)) {
    return strncmp(s1, s2, strlen(s1));
  } else {
    return 1;
  }
}
void httpserv_strstriplw(char *s) {
  if (!s)
    return;
  while (iswspace(s[0]) != 0) {
    for (int i = 1; i <= strlen(s); i++) {
      s[i - 1] = s[i];
    }
  }
}
void httpserv_strstriptw(char *s) {
  if (!s)
    return;
  while (iswspace(s[strlen(s) - 1]) != 0) {
    s[strlen(s) - 1] = '\0';
  }
}
void httpserv_strstripltw(char *s) {
  if (!s)
    return;
  httpserv_strstriplw(s);
  httpserv_strstriptw(s);
}
