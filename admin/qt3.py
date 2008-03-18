"""
Run scons -h to display the associated help, or look below ..
"""

import sys, os, re, types

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

def exists(env):
  return True

def detect_qt3(env):
  qtdir       = env['ARGS'].get('qtdir', None)
  qtincludes  = env['ARGS'].get('qtincludes', None)
  qtlibs      = env['ARGS'].get('qtlibs', None)

  ## Detect the qt library
  print "Checking for qt                   : ",
  if not qtdir:
    qtdir = os.getenv('QTDIR')

  if qtdir:
    print GREEN+qtdir+NORMAL
  elif env['PKGCONFIG']:
    qtdir = os.popen('pkg-config --variable=prefix qt-mt').read().strip()
    if len(qtdir):
      print GREEN+"found in " + NORMAL+BOLD + qtdir + NORMAL
    else:
      # QT not found
      print RED+"not found"+NORMAL
      print RED+"Please set QTDIR first (/usr/lib/qt3?)"+NORMAL
      env.Exit(1)
  else:
    # QT not found
    print RED+"not found"+NORMAL
    print RED+"Please set QTDIR first (/usr/lib/qt3?)"+NORMAL
    env.Exit(1)
  env['QTDIR'] = qtdir.strip()

  ## Find the necessary programs uic and moc
  print "Checking for uic                  : ",
  if env['PLATFORM'].startswith('win'):
    uic = qtdir + "/bin/uic.exe"
  else:
    uic = qtdir + "/bin/uic"

  if os.path.isfile(uic):
    print GREEN+"found as " + NORMAL+BOLD + uic + NORMAL
  else:
    uic = env.WhereIs("uic")
    if not uic:
      uic = ""
    if len(uic):
      print GREEN+"found as " + NORMAL+BOLD + uic + NORMAL
    else:
      print RED+"not found"+NORMAL
      env.Exit(1)
  env['QT_UIC'] = uic

  print "Checking for moc                  : ",
  if env['PLATFORM'].startswith('win'):
    moc = qtdir + "/bin/moc.exe"
  else:
    moc = qtdir + "/bin/moc"

  if os.path.isfile(moc):
    print GREEN + "found as " + NORMAL+BOLD + moc + NORMAL
  else:
    moc = env.WhereIs("moc")
    if not moc:
      moc = ""
    if len(moc):
      print GREEN + "found as " + NORMAL+BOLD + moc + NORMAL
    else:
      print RED + "not found" + NORMAL
      env.Exit(1)
  env['QT_MOC'] = moc

  ## check for the qt3 includes
  print "Checking for the qt includes      : ",
  if qtincludes and os.path.isfile(qtincludes + "/qlayout.h"):
    # The user told where to look for and it looks valid
    print GREEN + "ok " + NORMAL+BOLD + qtincludes + NORMAL
  else:
    if os.path.isfile(qtdir + "/include/qlayout.h"):
      # Automatic detection
      print GREEN + "ok " + qtdir + "/include/ " + NORMAL
      qtincludes = qtdir + "/include/"
    elif os.path.isfile("/usr/include/qt3/qlayout.h"):
      # Debian probably
      print GREEN+"found in " + NORMAL+BOLD + "/usr/include/qt3/ " + NORMAL
      qtincludes = "/usr/include/qt3"
    else:
      print RED + "not found" + NORMAL
      env.Exit(1)

  ## qt libs and includes
  env['QTINCLUDEPATH']=qtincludes
  if not qtlibs:
    qtlibs=qtdir+"/lib"
  env['QTLIBPATH']=qtlibs

def generate(env):
  """"Set up the qt and kde environment and builders - the moc part is difficult to understand """
  if env['HELP']:
    return

  import SCons.Defaults
  import SCons.Tool
  import SCons.Util

  ui_extensions = [".ui"]
  header_extensions = [".h", ".hxx", ".hpp", ".hh"]
  source_extensions = [".cpp", ".cxx", ".cc"]

  def find_file(filename, paths, node_factory):
    retval = None
    for dir in paths:
      node = node_factory(filename, dir)
      if node.rexists():
        return node
    return None

  class _Metasources:
    """ Callable class, which works as an emitter for Programs, SharedLibraries 
    and StaticLibraries."""
  
    def __init__(self, objBuilderName):
      self.objBuilderName = objBuilderName
  
    def __call__(self, target, source, env):
      """ Smart autoscan function. Gets the list of objects for the Program
      or Lib. Adds objects and builders for the special qt files. """
      try:
        if int(env.subst('$QT_AUTOSCAN')) == 0:
          return target, source
      except ValueError:
        pass

      try:
        qtdebug = int(env.subst('$QT_DEBUG'))
      except ValueError:
        qtdebug = 0

      # some shortcuts used in the scanner
      FS = SCons.Node.FS.default_fs
      splitext = SCons.Util.splitext
      objBuilder = getattr(env, self.objBuilderName)
  
      # some regular expressions:
      # Q_OBJECT detection
      q_object_search = re.compile(r'[^A-Za-z0-9]Q_OBJECT[^A-Za-z0-9]')

      # The following is kind of hacky to get builders working properly (FIXME) ??
      objBuilderEnv = objBuilder.env
      objBuilder.env = env
      mocBuilderEnv = env.Moc.env
      env.Moc.env = env

      # make a deep copy for the result; MocH objects will be appended
      out_sources = source[:]

      for obj in source:
        if not obj.has_builder():
          # binary obj file provided
          if qtdebug:
            print "qt: '%s' seems to be a binary. Discarded." % str(obj)
          continue
        cpp = obj.sources[0]
        if not splitext(str(cpp))[1] in source_extensions:
          if qtdebug:
            print "qt: '%s' is no cxx file. Discarded." % str(cpp)
          # c or fortran source
          continue
        #cpp_contents = comment.sub('', cpp.get_contents())
        cpp_contents = cpp.get_contents()

        h = None
        ui = None

        for ui_ext in ui_extensions:
          # try to find the ui file in the corresponding source directory
          uiname = splitext(cpp.name)[0] + ui_ext
          ui = find_file(uiname, (cpp.get_dir(),), FS.File)
          if ui:
            if qtdebug:
              print "qt: found .ui file of header" #% (str(h), str(cpp))
              #h_contents = comment.sub('', h.get_contents())
            break
  
        # if we have a .ui file, do not continue, it is automatically handled by Uic
        if ui:
          continue

        for h_ext in header_extensions:
          # try to find the header file in the corresponding source
          # directory
          hname = splitext(cpp.name)[0] + h_ext
          h = find_file(hname, (cpp.get_dir(),), FS.File)
          if h:
            if qtdebug:
              print "qt: Scanning '%s' (header of '%s')" % (str(h), str(cpp))
            #h_contents = comment.sub('', h.get_contents())
            h_contents = h.get_contents()
            break
  
        if not h and qtdebug:
          print "qt: no header for '%s'." % (str(cpp))
        if h and q_object_search.search(h_contents):
          # h file with the Q_OBJECT macro found -> add .moc or _moc.cpp file
          moc_cpp = None

          if env.has_key('NOMOCSCAN'):
            moc_cpp = env.Moc(h)
          else:
            reg = '\n\s*#include\s*("|<)'+splitext(cpp.name)[0]+'.moc("|>)'
            meta_object_search = re.compile(reg)
            if meta_object_search.search(cpp_contents):
              moc_cpp = env.Moc(h)
            else:
              moc_cpp = env.Moccpp(h)
              moc_o = objBuilder(moc_cpp)
              out_sources.append(moc_o)
          if qtdebug:
            print "qt: found Q_OBJECT macro in '%s', moc'ing to '%s'" % (str(h), str(moc_cpp[0]))
  
        if cpp and q_object_search.search(cpp_contents):
          print "error, bksys cannot handle cpp files with Q_OBJECT classes"

      # restore the original env attributes (FIXME)
      objBuilder.env = objBuilderEnv
      env.Moc.env = mocBuilderEnv

      return (target, out_sources)

  MetasourcesShared = _Metasources('SharedObject')
  MetasourcesStatic = _Metasources('StaticObject')

  CLVar = SCons.Util.CLVar
  splitext = SCons.Util.splitext
  Builder = SCons.Builder.Builder
  
  # Detect the environment - replaces ./configure implicitely and store the options into a cache
  from SCons.Options import Options
  cachefile=env['CACHEDIR']+'qt3.cache.py'
  opts = Options(cachefile)
  opts.AddOptions(
    ('QTDIR', 'root of qt directory'),
    ('QTLIBPATH', 'path to the qt libraries'),
    ('QTINCLUDEPATH', 'path to the qt includes'),
    ('QT_UIC', 'uic executable command'),
    ('QT_MOC', 'moc executable command'),
  )
  opts.Update(env)

  # reconfigure when things are missing
  if not env['HELP'] and (env['_CONFIGURE'] or not env.has_key('QTDIR')):
    detect_qt3(env)
    # finally save the configuration to the cache file
    opts.Save(cachefile, env)

  ## set default variables, one can override them in sconscript files
  env.Append(CPPPATH = [env['QTINCLUDEPATH']])
  env.Append(LIBPATH = [env['QTLIBPATH']])
  env.Append(LIBS    = ['qt-mt'])
  env.Append(CPPDEFINES = ['QT_THREAD_SUPPORT'])

  env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1

  env['QT_AUTOSCAN']          = 1
  env['QT_DEBUG']             = 0
  env['QT_UIC_HFLAGS']        = '-L $QTPLUGINS -nounload'
  env['QT_UIC_CFLAGS']        = '$QT_UIC_HFLAGS'
  env['QT_LIBS']              = 'qt-mt'
  env['QT_UICIMPLPREFIX']     = ''
  env['QT_UICIMPLSUFFIX']     = '.cpp'

  env['QT_MOCHPREFIX']        = ''
  env['QT_MOCHSUFFIX']        = '.moc'
  env['MSGFMT']               = 'msgfmt'

  ## ui file processing
  def uic_processing(target, source, env):
    inc_moc  = '#include "%s"\n' % target[2].name
    comp_h   = '$QT_UIC $QT_UIC_HFLAGS -o %s %s' % (target[0].path, source[0].path)
    comp_c   = '$QT_UIC $QT_UIC_CFLAGS -impl %s %s' % (target[0].path, source[0].path)
    comp_moc = '$QT_MOC -o %s %s' % (target[2].path, target[0].path)

    ret = env.Execute(comp_h)
    if ret:
      return ret

    ret = env.Execute( comp_c+" >> "+target[1].path )
    if ret:
      return ret

    dest = open( target[1].path, "a" )
    dest.write(inc_moc)
    dest.close()

    ret = env.Execute( comp_moc )
    return ret

  def uicEmitter(target, source, env):
    adjustixes = SCons.Util.adjustixes
    bs = SCons.Util.splitext(str(source[0].name))[0]
    bs = os.path.join(str(target[0].get_dir()),bs)
    # first target is automatically added by builder (.h file)
    if len(target) < 2:
      # second target is .cpp file
      target.append(adjustixes(bs,
        env.subst('$QT_UICIMPLPREFIX'),
        env.subst('$QT_UICIMPLSUFFIX')))
    if len(target) < 3:
      # third target is .moc file
      target.append(adjustixes(bs,
        env.subst('$QT_MOCHPREFIX'),
        env.subst('$QT_MOCHSUFFIX')))
    return target, source

  UIC_BUILDER = Builder(
    action     = uic_processing,
    emitter    = uicEmitter,
    suffix     = '.h',
    src_suffix = '.ui')

  ## moc file processing
  env['QT_MOCCOM'] = ('$QT_MOC -o ${TARGETS[0]} $SOURCE')

  MOC_BUILDER = Builder(
    action     = '$QT_MOCCOM',
    suffix     = '.moc',
    src_suffix = '.h')

  MOCCPP_BUILDER = Builder(
    action     = '$QT_MOCCOM',
    suffix     = '_moc.cpp',
    src_suffix = '.h')


  ## translation files builder
  TRANSFILES_BUILDER = Builder(
    action     = '$MSGFMT $SOURCE -o $TARGET',
    suffix     = '.gmo',
    src_suffix = '.po')

  ## register the builders
  env['BUILDERS']['Uic']       = UIC_BUILDER
  env['BUILDERS']['Moc']       = MOC_BUILDER
  env['BUILDERS']['Moccpp']    = MOCCPP_BUILDER
  env['BUILDERS']['Transfiles']= TRANSFILES_BUILDER

  static_obj, shared_obj = SCons.Tool.createObjBuilders(env)
  static_obj.src_builder.append('Uic')
  shared_obj.src_builder.append('Uic')
  static_obj.src_builder.append('Transfiles')
  shared_obj.src_builder.append('Transfiles')

  ## Find the files to moc, dcop, and link against kde and qt
  env.AppendUnique(PROGEMITTER = [MetasourcesStatic], SHLIBEMITTER=[MetasourcesShared], LIBEMITTER =[MetasourcesStatic])
