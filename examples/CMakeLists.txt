set(base_examples
    nrtv-p2p-example
    nrtv-variables-plot
)

foreach(
  example
  ${base_examples}
)
  build_lib_example(
    NAME ${example}
    SOURCE_FILES ${example}.cc
    LIBRARIES_TO_LINK
    ${libtraffic-extensions}
    ${libapplications}
    ${libpoint-to-point}
    ${libinternet}
    ${libnetwork}
  )
endforeach()
