{
  "targets": [
    {
      "target_name": "simple-example",
      "include_dirs": [ "/src" ],
      "sources": [ "src/addon.cc" ],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1
        }
      },
      "conditions": [
        ["OS=='win'", {
          "defines": [
            "_HAS_EXCEPTIONS=1"
          ]
        }]
      ]
    }
  ]
}