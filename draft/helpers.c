int helpers__ishexdigit(char ch) {
  return (('0' <= (ch) && (ch) <= '9') || ('a' <= (ch) && (ch) <= 'f') ||
          ('A' <= (ch) && (ch) <= 'F'));
}

int helpers__isdigit(char ch) { return ('0' <= (ch) && (ch) <= '9'); }

int helpers__isspace(char ch) {
  switch (ch) {
  case ' ':
  case '\f':
  case '\r':
  case '\n':
  case '\t':
    return 1;
  default:
    return 0;
  }
}
