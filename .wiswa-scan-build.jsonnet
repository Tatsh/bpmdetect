{
  checkout_options: { 'fetch-depth': 0 },
  configure_command: 'cmake -S . -B build-scan -DCMAKE_BUILD_TYPE=Debug',
  documentation_steps: [
    {
      name: 'Install dependencies',
      run: 'sudo apt-get update\nsudo apt-get install -y doxygen graphviz\n',
    },
    {
      name: 'Configure',
      run: 'cmake -DCMAKE_BUILD_TYPE=Release -B "${GITHUB_WORKSPACE}"/build --install-prefix "${GITHUB_WORKSPACE}"/build/prefix -DBUILD_DOCS_ONLY=ON',
    },
    {
      name: 'Build documentation',
      run: 'cmake --build "${GITHUB_WORKSPACE}"/build --verbose --config Release --target docs',
    },
  ],
  env: {},
  setup_steps: [
    {
      name: 'Install build dependencies',
      run: 'sudo apt-get update\nsudo apt-get install -y extra-cmake-modules libavcodec-dev libavformat-dev libavutil-dev libsoundtouch-dev\n',
    },
    {
      name: 'Install current CMake',
      run: 'pipx install cmake',
    },
    {
      name: 'Install Qt',
      uses: 'jurplel/install-qt-action@a9c63c7c123f3069cff414e7e482d95dfa9d8125',
      with: {
        modules: 'qtmultimedia',
      },
    },
  ],
  workflow_file: 'pages.yml',
}
