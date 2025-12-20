local utils = import 'utils.libjsonnet';

{
  security_policy_supported_versions: { '0.8.x': ':white_check_mark:' },
  authors+: [
    {
      'family-names': 'Trofimovich',
      'given-names': 'Sergei',
      email: 'slyich@gmail.com',
      name: '%s %s' % [self['given-names'], self['family-names']],
    },
    {
      'family-names': 'Sturmlechner',
      'given-names': 'Andreas',
      email: 'andreas.sturmlechner@gmail.com',
      name: '%s %s' % [self['given-names'], self['family-names']],
    },
    {
      'family-names': 'Parlant',
      'given-names': 'Nicolas',
      email: 'nicolas.parlant@parhuet.fr',
      name: '%s %s' % [self['given-names'], self['family-names']],
    },
    {
      'family-names': 'Sakmar',
      'given-names': 'Martin',
      email: 'martin.sakmar@gmail.com',
      name: '%s %s' % [self['given-names'], self['family-names']],
    },
  ],
  project_name: 'bpmdetect',
  version: '0.8.9',
  description: 'Automatic BPM (beats per minute) detection utility.',
  keywords: ['bpm', 'dj', 'music', 'tempo'],
  want_main: false,
  want_codeql: false,
  want_tests: false,
  copilot+: {
    intro: 'BPM Detect is an automatic BPM (beats per minute) detection utility.',
  },
  package_json+: {
    cspell+: {
      ignorePaths+: [
        '.docs/*.tags',
        '.docs/*.tag.xml',
      ],
    },
    scripts+: {
      'check-formatting': "clang-format -n src/*.cpp src/*.h && prettier -c . && markdownlint-cli2 '**/*.md' '#node_modules' '#vcpkg_installed'",
      'flatpak-build-install': 'flatpak run --command=flathub-build org.flatpak.Builder --install sh.tat.bpmdetect.yml',
      'flatpak-install': 'flatpak uninstall -y bpmdetect || true && flatpak install -y --user --reinstall flathub sh.tat.bpmdetect',
      'flatpak-lint': 'flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest sh.tat.bpmdetect.yml',
      'flatpak-run': 'flatpak run sh.tat.bpmdetect',
      'flatpak-uninstall': 'flatpak uninstall -y sh.tat.bpmdetect',
      format: 'cmake-format -i CMakeLists.txt src/CMakeLists.txt src/track/CMakeLists.txt src/widgets/CMakeLists.txt tests/CMakeLists.txt && clang-format -i src/*.cpp src/*.h && yarn prettier -w .',
    },
  },
  prettierignore+: ['*.desktop', '*.tags', 'src/icon.rc'],
  cz+: {
    commitizen+: {
      version_files+: [
        'man/bpmdetect.1',
        'sh.tat.bpmdetect.yml',
        'src/main.cpp',
      ],
    },
  },
  shared_ignore+: [
    '/.flatpak-builder/',
    '/build_fp/',
    '/repo/',
  ],
  vscode+: {
    c_cpp+: {
      configurations: [
        {
          cStandard: 'gnu23',
          compilerPath: '/usr/bin/gcc',
          cppStandard: 'gnu++23',
          includePath: [
            '${workspaceFolder}/src/**',
            '${workspaceFolder}/build/src/generated',
            '${workspaceFolder}/build/src/widgets/bpmdetect-widgets_autogen/include',
          ],
          name: 'Linux',
        },
      ],
    },
    launch+: {
      configurations: [
        {
          MIMode: 'gdb',
          args: ['/home/tatsh/Downloads/-template-with-cover-page.pdf'],
          cwd: '${workspaceFolder}',
          environment: [],
          externalConsole: false,
          name: 'Debug',
          program: '${workspaceFolder}/build/src/bpmdetect',
          request: 'launch',
          setupCommands: [
            {
              description: 'Enable pretty-printing for gdb',
              ignoreFailures: true,
              text: '-enable-pretty-printing',
            },
          ],
          stopAtEntry: false,
          type: 'cppdbg',
        },
        {
          MIMode: 'gdb',
          args: [],
          cwd: '${workspaceFolder}',
          environment: [],
          externalConsole: false,
          name: 'Debug Test',
          program: '${workspaceFolder}/build/tests/dlgtestbpm-test',
          request: 'launch',
          setupCommands: [
            {
              description: 'Enable pretty-printing for gdb',
              ignoreFailures: true,
              text: '-enable-pretty-printing',
            },
          ],
          stopAtEntry: false,
          type: 'cppdbg',
        },
      ],
    },
    settings+: {
      'cmake.configureArgs': ['-DBUILD_TESTS=ON', '-DCOVERAGE=ON'],
      'files.associations': {
        '*.moc': 'cpp',
        '*.ui': 'xml',
        'i18n/*.ts': 'xml',
      },
    },
  },
  // C++ only
  cmake+: {
    uses_qt: true,
  },
  project_type: 'c++',
  vcpkg+: {
    dependencies: [
      {
        name: 'ecm',
        'version>=': '6.7.0',
      },
      {
        features: ['network', 'widgets'],
        name: 'qtbase',
        'version>=': '6.8.3',
      },
      {
        features: [{ name: 'ffmpeg', platform: 'linux' }],
        name: 'qtmultimedia',
        'version>=': '6.8.3',
      },
      'ffmpeg',
      'soundtouch',
    ],
  },
  github+: {
    publish_winget: {
      identifier: 'Tatsh.BPMDetect',
      max_versions_to_keep: 1,
    },
  },
}
