#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "rule_parser.h"
#include "rule.h"
#include "packet_parser.h"
#include "../actions/alerts.h"
#include "../../../globals.h"
#include "../../utils.h"
static bool is_rule(const char *);
static bool is_comment(const char * line);
static void rstrip(char * );
static void syntax_error(const char * line, int line_no);
static void get_ruletype(const char * , struct rule *);
static void get_action(const char * , struct rule *);

// also the main config parser
void rule_library_parser(const char * alt_file){
  // alt_file will be determined elsewhere
  FILE * fp = fopen(alt_file,"r");
  if(fp == NULL){
    printf("Error opening configuration file: %s\n",alt_file);
    exit(EXIT_FAILURE);
  }
  size_t pos;
  size_t len = 0;
  char * line = NULL;
  
  while((pos = getline(&line,&len,fp)) != -1){
    line[strcspn(line,"\n")] = 0;
    if(is_comment(line) == true) continue;
    else if(strncmp(line,"strict_icmp_timestamp_req=",26) == 0){
      if(strcmp(line + 26,"YES") == 0) strict_icmp_timestamp_req = true;
      else strict_icmp_timestamp_req = false;
    }
    else if(strncmp(line,"strict_nmap_host_alive_check=",29) == 0){
      if(strcmp(line + 29, "YES") == 0) strict_nmap_host_alive_check = true;
      else strict_nmap_host_alive_check = false;
    }
    else if(strncmp(line,"clean_delay_in_mseconds=",24) == 0){
      
      if(strlen(line) == 24){
        printf("Clean delay needs a value\n");
        exit(EXIT_FAILURE);
      } 
      clean_delay = atoi(line + 24);
    }
    else if(strncmp(line,"use_mysql=",10) == 0){
      if(strcmp(line + 10,"YES") == 0) use_mysql = true;
      else use_mysql = false;
    }
    else if(strncmp(line,"mysql_user=",11) == 0){
      strcpy(mysql_user,line+11);

    }
    else if(strncmp(line,"mysql_password=",15) == 0){
      strcpy(mysql_password,line + 15);

    }
    else if(is_rule(line)){
      // printf("Parsing: %s\n",line);
      rule_parser(line);
    } 
  }
}
static bool is_rule(const char * line){
  return line[0] == '$' ? true : false;
}

static bool is_comment(const char * line){
  return strstr(line,"#") != NULL ? true : false;
}

static void rstrip(char * line){
  line[strcspn(line,"\n")] = 0;
}

static void syntax_error(const char * line, int line_no){
  printf("Syntax error at line %d: %s\n",line_no,line);
  exit(EXIT_FAILURE);
}

void rule_parser(const char * __filename){
  // + 1 for the $ at the beggining
  // also for the " " at the beggining when filename is given over network
  const char * filename = __filename + 1;
  FILE * fp = fopen(filename,"r");
  if(fp == NULL){
    printf("Error opening rule file: %s. Refusing to continue\n",filename);

    exit(EXIT_FAILURE);
  }
  printf("Parsing file %s\n",filename);
  size_t pos;
  size_t len = 0;
  char * line = NULL;
  
  while((pos = getline(&line,&len,fp)) != -1){
    if(is_comment(line))
      continue;
    rstrip(line);

    if(strcmp("\x00",line) == 0) continue;
    if(strstr(line,"alert") != NULL){
      line_parser(line);
    } 
  }
}


void line_parser(const char * line){
  int filled = 0;
	char * parser_line;
	char ruletype[32];
	char rulename[32];
	char target_chars[64];
	char alert_type[32];
	char protocol[10];
	char port[14];

	// the first par always needs to be "alert"
	int chars_parsed = 6;
  struct rule * __rule = &rules[num_rules++];
	while(filled != 5){
		parser_line = line + chars_parsed;
		char target[64];
		memset(&target,0,sizeof(target));
		strncpy(target,parser_line ,strloc(parser_line,' ') + 1);
		chars_parsed += strlen(target);

		if(filled == 0){
      get_action(target,__rule);
		} 
		else if(filled == 1){
      if(strcmp(target,"any") == 0) __rule->port == 0;
      else __rule->port = atoi(target);
		} 
		else if(filled == 2){
      get_protocol(target,__rule);
		}
		else if(filled == 3){
			char * name = target + 1;
			name[strcspn(name,"\"")] = 0;
			strcpy(__rule->rulename,name);
		}
		else if(filled == 4){
      get_ruletype(target,__rule);
		}
		else if(filled == 5){
			// memset(&target_chars,0,sizeof(target_chars));
			strncpy(__rule->rule_target,parser_line + 1,strlen(parser_line) - 5);
		}
		filled++;
	}
  printf("alert %s %s\n",__rule->rulename,__rule->rule_target);
}


static void get_protocol(const char * __line, struct rule * __rule){
  if(strncmp(__line,"TCP",3) == 0){
    __rule->protocol = R_TCP;  
  } 
  else if(strncmp(__line,"UDP",3) == 0){
    __rule->protocol = R_UDP;
  } 
  else if(strncmp(__line,"ICMP",4) == 0){
    __rule->protocol = R_ICMP;
  }
  else if(strncmp(__line,"ANY",3) == 0){
    __rule->protocol = R_ALL;
  } 
  else {
    printf("Unknown protocol type: %s. If you believe this protocol should be added"
    ", please contact me at cxmacolley@gmail.com\n"
    "For now, the best thing to do is to modify the rule parameter to say \"ALL\"\n",__line);
    exit(EXIT_FAILURE);
  }
}


static void get_ruletype(const char * __line, struct rule * __rule){
  if(strncmp(__line,"bit_match",9) == 0){
    __rule->pkt_parser = bit_match_parser;
    return;
  }

  else {
    printf("Unknown rule type: %s\n",__line);
    exit(EXIT_FAILURE);
  }
  
}


static void get_action(const char * __line, struct rule * __rule){
  if(strncmp(__line,"stdout",6) == 0){
    __rule->action = stdout_alert; 
    return;
  } 
  else {
    printf("Unknown action: %s\n",__line);
    exit(EXIT_FAILURE);
  }

}


void deny_conf_parser(char * file){
  FILE * fp = fopen(file,"r");
  if(fp == NULL){
    printf("Error opening %s for expl/implicit deny parsing\n",file);
    exit(EXIT_FAILURE);
  }
  char * line = NULL;
  size_t pos, len = 0;
  while((pos = getline(&line,&len,fp)) != -1){
    rstrip(line);
    if(is_comment(line)) continue;
    
    if(strcmp("\x00",line) == 0) continue;
    if(strstr(line,"ipv4")){
      struct blocked_ipv4 * temp = &blocked_ipv4_addrs[++blk_ipv4_len];
      char * ipv4_addr = line + 5;
      // printf("%s\n",ipv4_addr);
      strcpy(temp->ipv4_addr ,ipv4_addr);
    } 
  }
  
  if(line) free(line);
  
  /*
  for(int i = 0; i < blk_ipv4_len+1; i++){
    printf("Blocked IP address: %s\n",blocked_ipv4_addrs[i].ipv4_addr);
  }
  */
}


void host_mon_parser(){
  FILE * fp = fopen(default_host_conf,"r");
  if(fp == NULL){
    printf("Failed to open %s\n",default_host_conf);
    exit(EXIT_FAILURE);
  }
  fclose(fp);
}
