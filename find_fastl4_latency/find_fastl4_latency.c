#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

struct packet_info {
        long double time_epoch;
        unsigned int vlan_id;
        char ip_src[16];
        char ip_dst[16];
        unsigned int tcp_srcport;
        unsigned int tcp_dstport;
        unsigned long tcp_seq;
        unsigned long tcp_ack;
        char tcp_flags[11];
        unsigned int tcp_len;
        int has_searched;
};

int main(int argc, char *argv[]) {
        int vlan_inbound = 0;
        int vlan_outbound = 0;
        int packets_num = 0;

        int c;
        while ((c = getopt (argc, argv, "i:o:n:")) != -1) {
        switch (c) {
                case 'i':
                        vlan_inbound = atoi(optarg);
                        break;
                case 'o':
                        vlan_outbound = atoi(optarg);
                        break;
                case 'n':
                        packets_num = atoi(optarg);
                        break;
                case '?':
                        if (optopt == 'i' || optopt == 'o' || optopt == 'n')
                                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                        else if (isprint (optopt))
                                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                        else
                                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                        return 1;
                default:
                        exit(0);
        }
        }

        printf("inbound vlan = %d, outbound vlan = %d, number of packets = %d\n", vlan_inbound, vlan_outbound, packets_num);

        struct packet_info *packets_info = malloc(sizeof(struct packet_info) * packets_num);
        struct packet_info *packets_info_ptr = packets_info;
        char *line = NULL;
        size_t len = 0;
        int read;

        int i=1;
        while ((read = getline(&line, &len, stdin)) != -1) {
//                printf("line=%s", line);
                char *pch;
                pch = strtok (line, "\t");
                packets_info_ptr->time_epoch = atof(pch);
                pch = strtok (NULL, "\t");
                packets_info_ptr->vlan_id = atoi(pch);
                pch = strtok (NULL, "\t");
                strcpy(packets_info_ptr->ip_src, pch);
                pch = strtok (NULL, "\t");
                strcpy(packets_info_ptr->ip_dst, pch);
                pch = strtok (NULL, "\t");
                packets_info_ptr->tcp_srcport = atoi(pch);
                pch = strtok (NULL, "\t");
                packets_info_ptr->tcp_dstport = atoi(pch);
                pch = strtok (NULL, "\t");
                packets_info_ptr->tcp_seq = atol(pch);
                pch = strtok (NULL, "\t");
                packets_info_ptr->tcp_ack = atol(pch);
                pch = strtok (NULL, "\t");
                strcpy(packets_info_ptr->tcp_flags, pch);
                pch = strtok (NULL, "\t");
                packets_info_ptr->tcp_len = atoi(pch);
                packets_info_ptr->has_searched = 0;
                packets_info_ptr++;                
//                printf("i=%d\n", i++);
        }

        packets_info_ptr = packets_info;
        struct packet_info *packets_info_end = packets_info + packets_num - 1;
        struct packet_info *packets_info_search_ptr = packets_info;
//      printf("latency\tip_src\tip_dst\ttcp_srcport\ttcp_dstport\ttcp_seq\ttcp_ack\ttcp_flags\ttcp_len\n");
//      printf("-----------------------------------------------------------------------");
        for (int i = 0; i < packets_num; i++) {
                if (((packets_info_ptr->vlan_id == vlan_inbound) && (packets_info_ptr->has_searched == 0)) ||
                                ((packets_info_ptr->vlan_id == vlan_outbound) && (packets_info_ptr->has_searched == 0))) {
                        for (packets_info_search_ptr = packets_info_ptr + 1; 
                                        packets_info_search_ptr < packets_info_end;
                                        packets_info_search_ptr++) {
                                if ((((packets_info_ptr->vlan_id == vlan_inbound) && (packets_info_search_ptr->vlan_id == vlan_outbound)) ||
                                                ((packets_info_ptr->vlan_id == vlan_outbound) && (packets_info_search_ptr->vlan_id == vlan_inbound))) &&
                                                !strcmp(packets_info_search_ptr->ip_src, packets_info_ptr->ip_src) &&
                                                !strcmp(packets_info_search_ptr->ip_dst, packets_info_ptr->ip_dst) &&
                                                (packets_info_search_ptr->tcp_srcport == packets_info_ptr->tcp_srcport) &&
                                                (packets_info_search_ptr->tcp_dstport == packets_info_ptr->tcp_dstport) &&
                                                (packets_info_search_ptr->tcp_seq == packets_info_ptr->tcp_seq) &&
                                                (packets_info_search_ptr->tcp_ack == packets_info_ptr->tcp_ack) &&
                                                !strcmp(packets_info_search_ptr->tcp_flags, packets_info_ptr->tcp_flags) &&
                                                (packets_info_search_ptr->tcp_len == packets_info_ptr->tcp_len)
                                                ) {
                                        long double latency = packets_info_search_ptr->time_epoch - packets_info_ptr->time_epoch;
                                        printf("%Lf\t%s\t%s\t%d\t%d\t%lu\t%lu\t%s\t%d\n", latency,
                                                                packets_info_ptr->ip_src,
                                                                packets_info_ptr->ip_dst,
                                                                packets_info_ptr->tcp_srcport,
                                                                packets_info_ptr->tcp_dstport,
                                                                packets_info_ptr->tcp_seq,
                                                                packets_info_ptr->tcp_ack,
                                                                packets_info_ptr->tcp_flags,
                                                                packets_info_ptr->tcp_len);
                                        packets_info_search_ptr->has_searched = 1;
                                        break;               
                                }
                        }
                }
                packets_info_ptr++;
        }
        free(packets_info);
        exit(0);
}

