
#ifdef WINPCAP

#include <stdio.h>
#include <stdlib.h>

/* horrible hack, not sure what to do here about duplicate definitions */
#define htons winpcaphtons
#include "pcap.h"
#undef htons

#include "netdev.h"
#include "../uip/uip/uip.h"


extern uip_ipaddr_t default_hostaddr;

static pcap_t* handle = NULL;


struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};
int
initMAC(void)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t* devices, * initdev;
	struct pcap_addr *paddr;
	struct bpf_program fcode;
    unsigned int netmask;
	u16_t adr1;
    u16_t adr2;
    char buf[100];
	
    if (pcap_findalldevs(&devices, errbuf) == -1)
    {
        fprintf(stderr, "error pcap_findalldevs: %s\n", errbuf);
        return -1;
    }

    pcap_if_t* device;
	initdev = NULL;
    for(device = devices; device; device = device->next)
    {
        if (device->description)
        {
            printf(" (%s)\n", device->description);
			if ((strstr(device->description,"thernet")) ||
				(strstr(device->description,"ETHERNET")) ||
                (strstr(device->description,"Network")))
				initdev = device;
			paddr=device->addresses;
			if (paddr != NULL) {
				struct in_addr interface_addr;
				unsigned int i;
				interface_addr = ((struct sockaddr_in *)paddr->addr)->sin_addr;
				printf("init_pcap:    with address:");
				for (i=0;i<sizeof(interface_addr);i++)
					printf(" %d",((unsigned char *)&interface_addr)[i]);
				printf("\n");
			}
        }
        else
        {
            fprintf(stderr, "no device\n");
            return -1;
        }
    }

	if (initdev == NULL) {
		fprintf(stderr,"Could not figure out which device to open\n");
		return -1;
	}
	device = initdev;
	printf("Opening device (%s)\n",device->description);

    if (NULL == (handle= pcap_open_live(device->name
                                , 65536
                                , 0
                                , -1
                                , errbuf
                     )))
    {
        fprintf(stderr, "\nUnable to open the adapter. is not supported by WinPcap\n");
        pcap_freealldevs(devices);
        return -1;
    }
#if 1
	adr1=HTONS(default_hostaddr[0]);
	adr2=HTONS(default_hostaddr[1]);
     snprintf(buf, sizeof(buf), "dst host %d.%d.%d.%d",
	   (adr1 & 0xFF00) >> 8,
	   (adr1 & 0xFF),
	   (adr2 & 0xFF00) >> 8,
	   (adr2 & 0xFF));
#ifdef WPCAPDEBUG
	printf("filter is %s\n",buf);
#endif

    if (pcap_compile(handle, &fcode, buf, 1, netmask) < 0)
    {
        fprintf(stderr,"\nUnable to compile the packet filter. Check the syntax.\n");
        /* Free the device list */
        pcap_freealldevs(devices);
        return -1;
    }
    
    if (pcap_setfilter(handle, &fcode) < 0)
    {
        fprintf(stderr,"\nError setting the filter.\n");
        /* Free the device list */
        pcap_freealldevs(devices);
        return -1;
    }
#endif
    pcap_freealldevs(devices);
	return 0;
}

int asn=0;

u16_t MACRead()
{
    const u_char* packet;
    struct pcap_pkthdr* header;
    int res;

	res = pcap_next_ex(handle, &header, &packet);
	if (res == -1)
		printf("pcap read error %s\n",pcap_geterr(handle));
	if (res == 0)
		return 0;

    int readSize = (int)header->len >= UIP_BUFSIZE ? UIP_BUFSIZE : (int)header->len;
    memcpy(uip_buf, packet, readSize);
#ifdef WPCAPDEBUG
	printf("received packet=%d data=%c\n",readSize,((char *)uip_appdata)[0]); 
{
	int i;
	printf("read packet=");
	for (i=0;i<40;i++) {
		printf("%x ",((unsigned char *)uip_buf)[i]);
	}
	printf("\n");
}
#endif
#if 0
	if (asn == 0) {
		unsigned int i;
		asn = 1;
		for (i=0;i<6;i++)
			uip_ethaddr.addr[i] = ((unsigned char *)uip_buf)[i];
	}
#endif

	 return readSize;
}

u16_t MACWrite()
{
  int ret;
  char tmpbuf[UIP_BUFSIZE];
  int i;
  
  for(i = 0; i < (40 + UIP_LLH_LEN); i++) {
      tmpbuf[i] = uip_buf[i];
  }

  for(; i < uip_len; i++) {
      tmpbuf[i] = ((char *)uip_appdata)[i - (40 + UIP_LLH_LEN)];
  }
  ret = pcap_sendpacket(handle, tmpbuf, uip_len);
#ifdef WPCAPDEBUG
  printf("send packet=");
	for (i=0;i<14;i++) {
		printf("%x ",((unsigned char *)uip_buf)[i]);
	}
	printf("\n");
  printf("sent packet=%d data=%c\n",uip_len,((char *)uip_appdata)[0]);
 #endif

  if (ret == -1)
  {
      printf("sorry send error\n");
      exit(1);
  }
  return 0;
}

#endif  /* WINPCAP */
