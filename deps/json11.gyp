{ 'targets': [
    {
      'target_name': 'json11',
      'type': 'static_library',
      'sources': [
        "json11/json11.hpp",
        "json11/json11.cpp",
      ],
      'cflags': [
        '-fno-rtti',
        '-fno-exceptions',
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '.',
        ]
      },
      'include_dirs': [
        'json11',
      ],
    },
  ]
}
