local utils = import 'utils.libjsonnet';

(import 'defaults.libjsonnet') + {
  local top = self,
  // General settings

  // Shared
  github_username: 'Tatsh',
  security_policy_supported_versions: { '0.8.x': ':white_check_mark:' },
  authors: [
    {
      'family-names': 'Udvare',
      'given-names': 'Andrew',
      email: 'audvare@gmail.com',
      name: '%s %s' % [self['given-names'], self['family-names']],
    },
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
    }
  ],
  project_name: 'bpmdetect',
  version: '0.8.1',
  description: 'Automatic BPM (beats per minute) detection utility.',
  keywords: ['bpm', 'dj', 'music', 'tempo'],
  want_main: false,
  copilot: {
    intro: 'BPM Detect is an automatic BPM (beats per minute) detection utility.',
  },
  social+: {
    mastodon+: { id: '109370961877277568' },
  },
  prettierignore+: ['src/icon.rc'],

  // GitHub
  github+: {
    funding+: {
      ko_fi: 'tatsh2',
      liberapay: 'tatsh2',
      patreon: 'tatsh2',
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
      'libflac',
      'libmad',
      'libvorbis',
    ],
  },
}
