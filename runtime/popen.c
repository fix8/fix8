
/* cfengine for GNU

        Copyright (C) 1995
        Free Software Foundation, Inc.

   This file is part of GNU cfengine - written and maintained
   by Mark Burgess, Dept of Computing and Engineering, Oslo College,
   Dept. of Theoretical physics, University of Oslo

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

*/
/* $Id$
	$URL$
	modified by DD - we only need basic cfpopen, no cfengine error handling
	see http://www.cfengine.org/
*/

/*****************************************************************************/
/*                                                                           */
/* File: popen.c                                                             */
/*                                                                           */
/* Created: Tue Dec  2 02:14:16 1997                                         */
/*                                                                           */
/* popen replacement for POSIX 2 avoiding shell piggyback                    */
/*                                                                           */
/*****************************************************************************/

// #include "cf.defs.h"
// #include "cf.extern.h"
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define CF_BUFSIZE      4096
#define CF_MAXSHELLARGS 20

pid_t *CHILD;
int    MAXFD = 20; /* Max number of simultaneous pipes */

int SplitCommand(char *comm,char arg[CF_MAXSHELLARGS][CF_BUFSIZE]);

/*****************************************************************************/

FILE *cfpopen(char *command,char *type)

 { static char arg[CF_MAXSHELLARGS][CF_BUFSIZE];
   int i, argc, pd[2];
   char **argv;
   pid_t pid;
   FILE *pp = NULL;

if ((*type != 'r' && *type != 'w') || (type[1] != '\0'))
   {
   errno = EINVAL;
   return NULL;
   }

if (CHILD == NULL)   /* first time */
   {
   if ((CHILD = calloc(MAXFD,sizeof(pid_t))) == NULL)
      {
      return NULL;
      }
   }

if (pipe(pd) < 0)        /* Create a pair of descriptors to this process */
   {
   return NULL;
   }

if ((pid = fork()) == -1)
   {
   return NULL;
   }

//signal(SIGCHLD,SIG_DFL);

if (pid == 0)
   {
   switch (*type)
      {
      case 'r':

          close(pd[0]);        /* Don't need output from parent */

          if (pd[1] != 1)
             {
             dup2(pd[1],1);    /* Attach pp=pd[1] to our stdout */
             dup2(pd[1],2);    /* Merge stdout/stderr */
             close(pd[1]);
             }

          break;

      case 'w':

          close(pd[1]);

          if (pd[0] != 0)
             {
             dup2(pd[0],0);
             close(pd[0]);
             }
       }

   for (i = 0; i < MAXFD; i++)
      {
      if (CHILD[i] > 0)
         {
         close(i);
         }
      }

   argc = SplitCommand(command,arg);
   argv = (char **) malloc((argc+1)*sizeof(char *));

   if (argv == NULL)
      {
      //FatalError("Out of memory");
      }

   for (i = 0; i < argc; i++)
      {
      argv[i] = arg[i];
      }

   argv[i] = (char *) NULL;

   if (execv(arg[0],argv) == -1)
      {
      //snprintf(OUTPUT,CF_BUFSIZE,"Couldn't run %s",arg[0]);
      //CfLog(cferror,OUTPUT,"execv");
      }

   free((char *)argv);
   _exit(1);
   }
else
   {
   switch (*type)
      {
      case 'r':

          close(pd[1]);

          if ((pp = fdopen(pd[0],type)) == NULL)
             {
             return NULL;
             }
          break;

      case 'w':

          close(pd[0]);

          if ((pp = fdopen(pd[1],type)) == NULL)
             {
             return NULL;
             }
      }

   if (fileno(pp) >= MAXFD)
      {
      //snprintf(OUTPUT,CF_BUFSIZE,"File descriptor %d of child %d higher than MAXFD, check for defunct children", fileno(pp), pid);
      //CfLog(cferror,OUTPUT,"");
      return NULL;
      }
   else
      {
      CHILD[fileno(pp)] = pid;
      return pp;
      }
   }

return NULL; /* Cannot reach here */
}

/******************************************************************************/
/* Close commands                                                             */
/******************************************************************************/

int cfpclose(FILE *pp)

{ int fd; //, status, wait_result;
  pid_t pid;

if (CHILD == NULL)  /* popen hasn't been called */
   {
   return -1;
   }

fd = fileno(pp);

if (fd >= MAXFD)
   {
   //snprintf(OUTPUT,CF_BUFSIZE,"File descriptor %d of child higher than "
   //"MAXFD, check for defunct children", fd);
   //CfLog(cferror,OUTPUT,"");
   fclose(pp);
   return -1;
   }

if ((pid = CHILD[fd]) == 0)
   {
   return -1;
   }

CHILD[fd] = 0;

if (fclose(pp) == EOF)
   {
   return -1;
   }

return 0;

#if 0
#ifdef HAVE_WAITPID

while(waitpid(pid,&status,0) < 0)
   {
   if (errno != EINTR)
      {
      return -1;
      }
   }

return status;

#else

while ((wait_result = wait(&status)) != pid)
   {
   if (wait_result <= 0)
      {
      //snprintf(OUTPUT,CF_BUFSIZE,"Wait for child failed\n");
      //CfLog(cfinform,OUTPUT,"wait");
      return -1;
      }
   }

if (WIFSIGNALED(status))
   {
   return -1;
   }

if (! WIFEXITED(status))
   {
   return -1;
   }

return (WEXITSTATUS(status));
#endif
#endif
}

/*******************************************************************/
/* Command exec aids                                               */
/*******************************************************************/

int SplitCommand(char *comm,char arg[CF_MAXSHELLARGS][CF_BUFSIZE])

{ char *sp;
  int i = 0;
  int commlen = strlen(comm);

for (sp = comm; sp < comm+commlen; sp++)
   {
   if (i >= CF_MAXSHELLARGS-1)
      {
      //CfLog(cferror,"Too many arguments in embedded script","");
      //FatalError("Use a wrapper");
      }

   while (*sp == ' ' || *sp == '\t')
      {
      sp++;
      }

   switch (*sp)
      {
      case '\0': return(i-1);

      case '\"': sscanf (++sp,"%[^\"]",arg[i]);
          break;
      case '\'': sscanf (++sp,"%[^\']",arg[i]);
          break;
      case '`':  sscanf (++sp,"%[^`]",arg[i]);
          break;
      default:   sscanf (sp,"%s",arg[i]);
          break;
      }

   sp += strlen(arg[i]);
   i++;
   }

 return (i);
}



