/* gpgconf.c - Configuration utility for GnuPG
 *	Copyright (C) 2003 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include <config.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpgconf.h"
#include "i18n.h"

/* Constants to identify the commands and options. */
enum cmd_and_opt_values
  {
    aNull = 0,
    oDryRun	= 'n',
    oOutput	= 'o',
    oQuiet      = 'q',
    oVerbose	= 'v',
    oRuntime    = 'r',
    oComponent  = 'c',
    oNoVerbose	= 500,
    oHomedir,

    aListComponents,
    aListOptions,
    aChangeOptions,
    aApplyDefaults,
    aCheckConfig

  };


/* The list of commands and options. */
static ARGPARSE_OPTS opts[] =
  {
    { 300, NULL, 0, N_("@Commands:\n ") },
    
    { aListComponents, "list-components", 256, N_("list all components") },
    { aListOptions, "list-options", 256, N_("|COMPONENT|list options") },
    { aChangeOptions, "change-options", 256, N_("|COMPONENT|change options") },
    { aApplyDefaults, "apply-defaults", 256,
      N_("apply global default values") },
    { aCheckConfig,   "check-config", 256,
      N_("check global configuration file") },

    { 301, NULL, 0, N_("@\nOptions:\n ") },
    
    { oOutput, "output",    2, N_("use as output file") },
    { oVerbose, "verbose",  0, N_("verbose") },
    { oQuiet, "quiet",      0, N_("quiet") },
    { oDryRun, "dry-run",   0, N_("do not make any changes") },
    { oRuntime, "runtime",  0, N_("activate changes at runtime, if possible") },
    /* hidden options */
    { oNoVerbose, "no-verbose",  0, "@"},
    {0}
  };


/* Print usage information and and provide strings for help. */
static const char *
my_strusage( int level )
{
  const char *p;

  switch (level)
    {
    case 11: p = "gpgconf (GnuPG)";
      break;
    case 13: p = VERSION; break;
    case 17: p = PRINTABLE_OS_NAME; break;
    case 19: p = _("Please report bugs to <" PACKAGE_BUGREPORT ">.\n");
      break;
    case 1:
    case 40: p = _("Usage: gpgconf [options] (-h for help)");
      break;
    case 41:
      p = _("Syntax: gpgconf [options]\n"
            "Manage configuration options for tools of the GnuPG system\n");
      break;

    default: p = NULL; break;
    }
  return p;
}


/* Initialize the gettext system. */
static void
i18n_init(void)
{
#ifdef USE_SIMPLE_GETTEXT
  set_gettext_file (PACKAGE_GT);
#else
# ifdef ENABLE_NLS
  setlocale (LC_ALL, "" );
  bindtextdomain (PACKAGE_GT, LOCALEDIR);
  textdomain (PACKAGE_GT);
# endif
#endif
}


/* gpgconf main. */
int
main (int argc, char **argv)
{
  ARGPARSE_ARGS pargs;
  const char *fname;
  int no_more_options = 0;
  enum cmd_and_opt_values cmd = 0;

  set_strusage (my_strusage);
  log_set_prefix ("gpgconf", 1);

  i18n_init();

  /* Parse the command line. */
  pargs.argc  = &argc;
  pargs.argv  = &argv;
  pargs.flags =  1;  /* Do not remove the args.  */
  while (!no_more_options && optfile_parse (NULL, NULL, NULL, &pargs, opts))
    {
      switch (pargs.r_opt)
        {
        case oOutput:    opt.outfile = pargs.r.ret_str; break;
	case oQuiet:     opt.quiet = 1; break;
        case oDryRun:    opt.dry_run = 1; break;
        case oRuntime:
	  opt.runtime = 1;
	  break;
        case oVerbose:   opt.verbose++; break;
        case oNoVerbose: opt.verbose = 0; break;

        case aListComponents:
        case aListOptions:
        case aChangeOptions:
        case aApplyDefaults:
        case aCheckConfig:
	  cmd = pargs.r_opt;
	  break;

        default: pargs.err = 2; break;
	}
    }

  if (log_get_errorcount (0))
    exit (2);
  
  fname = argc ? *argv : NULL;
  
  switch (cmd)
    {
    case aListComponents:
    default:
      /* List all components. */
      gc_component_list_components (stdout);
      break;

    case aListOptions:
    case aChangeOptions:
      if (!fname)
	{
	  fputs (_("usage: gpgconf [options] "), stderr);
	  putc ('\n',stderr);
	  fputs (_("Need one component argument"), stderr);
	  putc ('\n',stderr);
	  exit (2);
	}
      else
	{
	  int idx = gc_component_find (fname);
	  if (idx < 0)
	    {
	      fputs (_("Component not found"), stderr);
	      putc ('\n', stderr);
	      exit (1);
	    }
	  gc_component_retrieve_options (idx);
          if (gc_process_gpgconf_conf (NULL, 1, 0))
            exit (1);
	  if (cmd == aListOptions)
	    gc_component_list_options (idx, stdout);
	  else
            gc_component_change_options (idx, stdin);
	}
      break;

    case aCheckConfig:
      if (gc_process_gpgconf_conf (fname, 0, 0))
        exit (1);
      break;

    case aApplyDefaults:
      if (fname)
	{
	  fputs (_("usage: gpgconf [options] "), stderr);
	  putc ('\n',stderr);
	  fputs (_("No argument allowed"), stderr);
	  putc ('\n',stderr);
	  exit (2);
	}
      gc_component_retrieve_options (-1);
      if (gc_process_gpgconf_conf (NULL, 1, 1))
        exit (1);
      break;

    }
  
  return 0; 
}



