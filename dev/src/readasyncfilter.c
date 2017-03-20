/**
 * Sample program that reads tags in the background and prints the
 * tags found that match a certain filter.
 * @file readasyncfilter.c
 */

#include <tm_reader.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#ifndef WIN32
#include <unistd.h>
#endif

/* Enable this to use transportListener */
#ifndef USE_TRANSPORT_LISTENER
#define USE_TRANSPORT_LISTENER 0
#endif

#define usage() {errx(1, "Please provide reader URL, such as:\n"\
                         "tmr:///com4 or tmr:///com4 --ant 1,2\n"\
                         "tmr://my-reader.example.com or tmr://my-reader.example.com --ant 1,2\n");}

static int matched;
static int nonMatched;
// char toMatch[128];

void errx(int exitval, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);

  exit(exitval);
}

void checkerr(TMR_Reader* rp, TMR_Status ret, int exitval, const char *msg)
{
  if (TMR_SUCCESS != ret)
  {
    errx(exitval, "Error %s: %s\n", msg, TMR_strerr(rp, ret));
  }
}

void serialPrinter(bool tx, uint32_t dataLen, const uint8_t data[],
                   uint32_t timeout, void *cookie)
{
  FILE *out = cookie;
  uint32_t i;

  fprintf(out, "%s", tx ? "Sending: " : "Received:");
  for (i = 0; i < dataLen; i++)
  {
    if (i > 0 && (i & 15) == 0)
      fprintf(out, "\n         ");
    fprintf(out, " %02x", data[i]);
  }
  fprintf(out, "\n");
}

void stringPrinter(bool tx,uint32_t dataLen, const uint8_t data[],uint32_t timeout, void *cookie)
{
  FILE *out = cookie;

  fprintf(out, "%s", tx ? "Sending: " : "Received:");
  fprintf(out, "%s\n", data);
}

void parseAntennaList(uint8_t *antenna, uint8_t *antennaCount, char *args)
{
  char *token = NULL;
  char *str = ",";
  uint8_t i = 0x00;
  int scans;

  /* get the first token */
  if (NULL == args)
  {
    fprintf(stdout, "Missing argument\n");
    usage();
  }

  token = strtok(args, str);
  if (NULL == token)
  {
    fprintf(stdout, "Missing argument after %s\n", args);
    usage();
  }

  while(NULL != token)
  {
    scans = sscanf(token, "%"SCNu8, &antenna[i]);
    if (1 != scans)
    {
      fprintf(stdout, "Can't parse '%s' as an 8-bit unsigned integer value\n", token);
      usage();
    }
    i++;
    token = strtok(NULL, str);
  }
  *antennaCount = i;
}

void countMatchListener(TMR_Reader *reader, const TMR_TagReadData *t, void *cookie);
void exceptionCallback(TMR_Reader *reader, TMR_Status error, void *cookie);

int main(int argc, char *argv[])
{

#ifndef TMR_ENABLE_BACKGROUND_READS
  errx(1, "This sample requires background read functionality.\n"
          "Please enable TMR_ENABLE_BACKGROUND_READS in tm_config.h\n"
          "to run this codelet\n");
  return -1;
#else

  TMR_Reader r, *rp;
  TMR_Status ret;
  TMR_Region region;
  TMR_ReadPlan plan;
  TMR_ReadListenerBlock rlb;
  TMR_ReadExceptionListenerBlock reb;
  uint8_t *antennaList = NULL;
  uint8_t buffer[20];
  uint8_t i;
  uint8_t antennaCount = 0x0;
  TMR_String model;
  char str[64];
#if USE_TRANSPORT_LISTENER
  TMR_TransportListenerBlock tb;
#endif

  if (argc < 2)
  {
    usage();
  }
  
  for (i = 2; i < argc; i+=2)
  {
    if(0x00 == strcmp("--ant", argv[i]))
    {
      if (NULL != antennaList)
      {
        fprintf(stdout, "Duplicate argument: --ant specified more than once\n");
        usage();
      }
      parseAntennaList(buffer, &antennaCount, argv[i+1]);
      antennaList = buffer;
    }
    else
    {
      fprintf(stdout, "Argument %s is not recognized\n", argv[i]);
      usage();
    }
  }

  rp = &r;
  ret = TMR_create(rp, argv[1]);
  checkerr(rp, ret, 1, "creating reader");

#if USE_TRANSPORT_LISTENER

  if (TMR_READER_TYPE_SERIAL == rp->readerType)
  {
    tb.listener = serialPrinter;
  }
  else
  {
    tb.listener = stringPrinter;
  }
  tb.cookie = stdout;

  TMR_addTransportListener(rp, &tb);
#endif

  ret = TMR_connect(rp);
  checkerr(rp, ret, 1, "connecting reader");

  region = TMR_REGION_NONE;
  ret = TMR_paramGet(rp, TMR_PARAM_REGION_ID, &region);
  checkerr(rp, ret, 1, "getting region");

  printf("Setting region to EU3.\n");
  region = TMR_REGION_EU3;
  ret = TMR_paramSet(rp, TMR_PARAM_REGION_ID, &region);
  checkerr(rp, ret, 1, "setting region");

  {
    printf("Setting hoptable to 866300.\n");
    uint32_t hoptablelist[1] = {866300};//hoptable: 866300, 866900, 867500, 865700, 
    TMR_uint32List hoptable;
    hoptable.list = hoptablelist;
    hoptable.len = 1;
    hoptable.max = 1;
    ret = TMR_paramSet(rp, TMR_PARAM_REGION_HOPTABLE, &hoptable);
    checkerr(rp, ret, 1, "setting hoptable");
  }

  { // Setting Tari to 25uS
  TMR_GEN2_Tari tari = TMR_GEN2_TARI_25US; // should be 25 ????
  printf("Setting Tari to 25us\n");
  ret = TMR_paramSet(rp, TMR_PARAM_GEN2_TARI , &tari);      
  checkerr(rp, ret, 1, "setting tari");
  }

  { // Setting miller to 8
  TMR_GEN2_TagEncoding miller = TMR_GEN2_MILLER_M_8;
  printf("Setting miller to 8\n");
  ret = TMR_paramSet(rp, TMR_PARAM_GEN2_TAGENCODING , &miller);      
  checkerr(rp, ret, 1, "setting miller");
  }


  { // Setting Session to 1
  TMR_GEN2_Session session = TMR_GEN2_SESSION_S0;
  printf("Setting Session to 1\n");
  ret = TMR_paramSet(rp, TMR_PARAM_GEN2_SESSION , &session);      
  checkerr(rp, ret, 1, "setting Session");
  }
  {
    TMR_GEN2_Target targetvalue = TMR_GEN2_TARGET_A;
    printf("Setting Target to AB\n");
    ret = TMR_paramSet(rp, TMR_PARAM_GEN2_TARGET , &targetvalue); 
    checkerr(rp, ret, 1, "setting Target");
  }
  {
    TMR_GEN2_LinkFrequency blf = TMR_GEN2_LINKFREQUENCY_250KHZ;
    printf("Setting BLF 250KHz \n");
    ret = TMR_paramSet(rp, TMR_PARAM_GEN2_BLF , &blf); 
    checkerr(rp, ret, 1, "setting backscatter link frequency");
  }
  { // Setting Q static 0
  TMR_SR_GEN2_QStatic Q;
  uint8_t initQ = 0;
  Q.initialQ = initQ;
  printf("Setting Q Static to 0\n");
  ret = TMR_paramSet(rp, TMR_PARAM_GEN2_Q , &Q);      
  checkerr(rp, ret, 1, "setting Q Static");
  }
  model.value = str;
  model.max = 64;
  TMR_paramGet(rp, TMR_PARAM_VERSION_MODEL, &model);
  if (((0 == strcmp("Sargas", model.value)) || (0 == strcmp("M6e Micro", model.value)) ||(0 == strcmp("M6e Nano", model.value)))
    && (NULL == antennaList))
  {
    fprintf(stdout, "Module doesn't has antenna detection support please provide antenna list\n");
    usage();
  }

  /**
  * for antenna configuration we need two parameters
  * 1. antennaCount : specifies the no of antennas should
  *    be included in the read plan, out of the provided antenna list.
  * 2. antennaList  : specifies  a list of antennas for the read plan.
  **/ 

  // initialize the read plan 
  ret = TMR_RP_init_simple(&plan, antennaCount, antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
  checkerr(rp, ret, 1, "initializing the  read plan");

  /* Commit read plan */
  ret = TMR_paramSet(rp, TMR_PARAM_READ_PLAN, &plan);
  checkerr(rp, ret, 1, "setting read plan");

  {
    /* false -- Each antenna gets a separate record
     * true -- All antennas share a single record */
    bool value = false;
    ret = TMR_paramSet(rp, TMR_PARAM_TAGREADDATA_UNIQUEBYANTENNA,&value);
    checkerr(rp, ret, 1, "setting uniqueByAntenna");
  }
  rlb.listener = countMatchListener;
  rlb.cookie = NULL;

  reb.listener = exceptionCallback;
  reb.cookie = NULL;

  // toMatch = "300833B2DDD9014000000000";

  ret = TMR_addReadListener(rp, &rlb);
  checkerr(rp, ret, 1, "adding read listener");

  ret = TMR_addReadExceptionListener(rp, &reb);
  checkerr(rp, ret, 1, "adding exception listener");

  ret = TMR_startReading(rp);
  checkerr(rp, ret, 1, "starting reading");
  
#ifndef WIN32
  sleep(5);
#else
  Sleep(5000);
#endif

  ret = TMR_stopReading(rp);
  checkerr(rp, ret, 1, "stopping reading");

  ret = TMR_removeReadListener(rp, &rlb);
  checkerr(rp, ret, 1, "removing read listener");

  // Print results of search, accumulated in listener object
  printf("Matching tags: %d\n", matched);
  printf("Non-matching tags: %d\n", nonMatched);

  TMR_destroy(rp);
  return 0;

#endif /* TMR_ENABLE_BACKGROUND_READS */
}


void
countMatchListener(TMR_Reader *reader, const TMR_TagReadData *t, void *cookie)
{
  char epcStr[128];
  if ( (0 < t->tag.epcByteCount))
  {
    TMR_bytesToHex(t->tag.epc, t->tag.epcByteCount, epcStr);

    // printf("Background read: %s\n", epcStr);
    if(0 == strcmp("300833B2DDD9014000000000", epcStr))
    {
      printf("Background read: %s\n", epcStr);
      printf(" ant:%d ", t->antenna);
      matched ++;
    }

  }
}

void
exceptionCallback(TMR_Reader *reader, TMR_Status error, void *cookie)
{
  fprintf(stdout, "Error:%s\n", TMR_strerr(reader, error));
}

