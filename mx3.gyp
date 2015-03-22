{
  'targets': [
    {
      'target_name': 'libmx3',
      'type': 'static_library',
      'conditions': [],
      'dependencies': [
        'deps/json11.gyp:json11',
        'deps/sqlite3.gyp:sqlite3',
      ],
      'sources': [
        # just automatically include all cpp and hpp files in src/ (for now)
        # '<!' is shell expand
        # '@' is to splat the arguments into list items
        "<!@(python glob.py src/ *.cpp *.hpp)",
      ],
      'include_dirs': [
        'include',
      ],
      'all_dependent_settings': {
        'include_dirs': [
          'include',
          'deps',
        ],
      },
    },
    {
      'target_name': 'libmx3_objc',
      'type': 'static_library',
      'conditions': [],
      'dependencies': [
        'deps/djinni/support-lib/support_lib.gyp:djinni_objc',
        'libmx3',
      ],
      'sources': [
        '<!@(python glob.py objc *.mm *.h *.m)',
      ],
      'sources!': ['play.m'],
      'include_dirs': [
        'include',
        'objc',
      ],
      'all_dependent_settings': {
        'include_dirs': [
          'include',
          'objc',
        ],
      },
    },
    {
      'target_name': 'libmx3_android',
      'android_unmangled_name': 1,
      'type': 'shared_library',
      'dependencies': [
        'deps/djinni/support-lib/support_lib.gyp:djinni_jni',
        'libmx3',
      ],
      'ldflags' : [ '-llog' ],
      'sources': [
        '<!@(python glob.py android/jni *.cpp *.hpp)',
        '<!@(python glob.py android/jni_gen *.cpp *.hpp)',
      ],
      'include_dirs': [
        'include',
        'src/interface',
      ],
      'all_dependent_settings': {
        'include_dirs': [
          'include',
          'src/interface',
        ],
      },
    },
    {
      'target_name': 'play_objc',
      'type': 'executable',
      'dependencies': ['libmx3_objc'],
      # I'm not sure why you have to specify libc++ when you build this :(
      'libraries': [
        'libc++.a',
      ],
      'sources': [
        'objc/play.m',
      ],
    },
    {
      'target_name': 'test',
      'type': 'executable',
      'dependencies': [
        'libmx3',
        'deps/gtest.gyp:gtest',
      ],
      'cflags_cc!': [ '-Werror', '-Wextra' ],
      'xcode_settings': {
        'OTHER_CPLUSPLUSFLAGS!' : ['-Werror', '-Wextra'],
      },
      'include_dirs': [
        '.',
        'test',
      ],
      'sources': [
        '<!@(python glob.py test *.cpp *.hpp)',
      ]
    },
    {
      'target_name': 'xctest_runner',
      'type': 'loadable_module',
      'mac_xctest_bundle': 1,
      'cflags_cc!': [ '-Werror', '-Wextra' ],
      'xcode_settings': {
        'OTHER_CPLUSPLUSFLAGS!' : ['-Werror', '-Wextra'],
      },
      'dependencies': [
        'libmx3',
        'deps/gtest.gyp:gtest',
      ],
      'include_dirs': [
        '.',
        'test',
      ],
      'sources': [
        '<!@(python glob.py test *.cpp *.hpp)',
        'deps/xcode-googletest/Bundle/GoogleTests.mm',
      ]
    },
  ],
}
