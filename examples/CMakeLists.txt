macro(mahi_com_example target)
    # create executable
    add_executable(${target} "ex_${target}.cpp")
    # set dependencies
    target_link_libraries(${target} mahi::com)
    # add install rule
    install(TARGETS ${target}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    set_target_properties(${target} PROPERTIES FOLDER "Examples")
    set_target_properties(${target} PROPERTIES DEBUG_POSTFIX -d)
endmacro(mahi_com_example)

mahi_com_example(chat)
mahi_com_example(comms_server)
mahi_com_example(melnet)
mahi_com_example(melscope)
mahi_com_example(melshare)
mahi_com_example(serial)
mahi_com_example(shared_memory)
mahi_com_example(udp)
