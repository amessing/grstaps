#Macro:      make_outputs
#Parameters: rootdir      Used for making relative paths
#            files        Files to be copied
#            outdir       Destination directory for the files
#Returns:    outputs      Destination file paths
#
#Use in conjunction with a custom target and add_dependencies to copy files
# when they are changed.
#
#Example usage:
# make_outputs("/home/user/project/data"
#              "${data_files}"
#              ${CMAKE_CURRENT_BINARY_DIR}/data
#              outputs)
# add_executable(my_exe ... ${outputs})
macro(make_outputs rootdir files outdir outputs)
    set(_outputs "")
    foreach(file ${files})
        if((NOT (IS_ABSOLUTE ${file})) AND (EXISTS "${rootdir}/${file}"))
            set(file "${rootdir}/${file}") #File is not absolute, but is relative to rootdir
        elseif((NOT (IS_ABSOLUTE ${file})) AND (NOT (EXISTS "${rootdir}/${file}")))
            message(SEND_ERROR "Filepath '${file}' is not absolute and is not relative to '${rootdir}'")
            return()
        endif()
        file(RELATIVE_PATH file_name ${rootdir} ${file})

        add_custom_command(
            OUTPUT ${outdir}/${file_name}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${file}
                ${outdir}/${file_name}
            DEPENDS ${file}
            COMMENT "Copying ${file_name}"
            VERBATIM)
        list(APPEND _outputs ${outdir}/${file_name})
    endforeach()
    set(${outputs} ${_outputs})
endmacro()