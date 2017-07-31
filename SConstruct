#! /usr/bin/env python

"""
help       -> scons -h
compile    -> scons
clean      -> scons -c
install    -> scons install
uninstall  -> scons -c install
configure  -> scons configure prefix=/tmp/ita debug=full extraincludes=/usr/local/include:/tmp/include prefix=/usr/local
"""

import sys, os

if not sys.platform.startswith('win'):
  BOLD   ="\033[1m"
  RED    ="\033[91m"
  GREEN  ="\033[92m"
  YELLOW ="\033[93m" # unreadable on white backgrounds
  CYAN   ="\033[96m"
  NORMAL ="\033[0m"
else:
  BOLD   =""
  RED    =""
  GREEN  =""
  YELLOW =""
  CYAN   =""
  NORMAL =""

###################################################################
# SCRIPTS FOR BUILDING THE TARGETS
###################################################################

SConscript(File('src/SConscript'), build_dir=Dir('build'), duplicate=0)


###################################################################
# CONVENIENCE FUNCTIONS TO EMULATE 'make dist' and 'make distclean'
###################################################################
"""
### To make a tarball of your masterpiece, use 'scons dist'
if 'dist' in COMMAND_LINE_TARGETS:
  
  ## The target scons dist requires the python module shutil which is in 2.3
  env.EnsurePythonVersion(2, 3)

  import os
  APPNAME = 'bpmdetect'
  VERSION = os.popen("cat VERSION").read().rstrip()
  FOLDER  = APPNAME+'-'+VERSION
  ARCHIVE = FOLDER+'.tar.gz'

  import shutil
  import glob

  ## check if the temporary directory already exists
  if os.path.isdir(FOLDER):
    shutil.rmtree(FOLDER)
  if os.path.isfile(ARCHIVE):
    os.remove(ARCHIVE)

  ## create a temporary directory
  startdir = os.getcwd()
  shutil.copytree(startdir, FOLDER)

  ## remove our object files first
  os.popen("find "+FOLDER+" -name \"*cache*\" | xargs rm -rf")
  os.popen("find "+FOLDER+" -name \"*.pyc\" | xargs rm -f")
  #os.popen("pushd %s && scons -c " % FOLDER) # TODO

  ## CVS cleanup
  os.popen("find "+FOLDER+" -name \"CVS\" | xargs rm -rf")
  os.popen("find "+FOLDER+" -name \".cvsignore\" | xargs rm -rf")

  ## Subversion cleanup
  os.popen("find %s -name .svn -type d | xargs rm -rf" % FOLDER)

  ## GNU Arch cleanup
  os.popen("find "+FOLDER+" -name \"{arch}\" | xargs rm -rf")
  os.popen("find "+FOLDER+" -name \".arch-i*\" | xargs rm -rf")

  ## Create the tarball (coloured output)
  print "\033[92m"+"Writing archive "+ARCHIVE+"\033[0m"
  os.popen("tar czf "+ARCHIVE+" "+FOLDER)

  ## Remove the temporary directory
  if os.path.isdir(FOLDER):
    shutil.rmtree(FOLDER)

  env.Default(None)
  env.Exit(0)

### Emulate "make distclean"
if 'distclean' in COMMAND_LINE_TARGETS:
  ## Remove the cache directory
  import os, shutil
  if os.path.isdir(env['CACHEDIR']):
    shutil.rmtree(env['CACHEDIR'])
  os.popen("find . -name \"*.pyc\" | xargs rm -rf")
"""
