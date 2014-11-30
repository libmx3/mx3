{ 'targets': [
    {
      'target_name': 'sqlite3',
      'type': 'static_library',
      'sources': [
        "sqlite3/sqlite3.c"
      ],
      'defines': [
        'HAVE_USLEEP=1',
      ],
      'cflags': [
        '-DHAVE_USLEEP=1',
        '-Wno-unused-const-variable',
      ],
      'xcode_settings': {
        'OTHER_CFLAGS' : ['<@(_cflags)'],
        'OTHER_CFLAGS!' : ['-Wextra', '-Werror'],
      },
      'all_dependent_settings': {
        'include_dirs': [
          '.',
          'sqlite3',
        ]
      },
      'include_dirs': [
        'sqlite3',
      ],
    },
  ]
}
