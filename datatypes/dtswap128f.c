from)
{
  DTTTYPE to;
  unsigned char *fp=((unsigned char *)&from)+15,*tp=(unsigned char *)&to;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  *tp++=*fp--;
  return to;
}

  	

