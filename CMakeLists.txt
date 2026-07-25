cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/import-std")
include(enable_import_std)

enable_experimental_std()
project("parallel-square_calculation" LANGUAGES CXX)
enable_cxx_modules(STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)

add_executable("parallel-square_calculation"
		"src/main.cpp"
		"src/main_templates/multiprocess/daemon.cpp"
		"src/main_templates/multiprocess/data_management.cpp"
		"src/main_templates/multiprocess/main_logic.cpp"
		"src/main_templates/multiprocess/process_management.cpp"
		"src/main_templates/multiprocess/task_management.cpp"
		"src/main_templates/singleprocess/death_event.cpp"
		"src/main_templates/singleprocess/input.cpp"
)
target_sources("parallel-square_calculation" PUBLIC FILE_SET modules TYPE CXX_MODULES FILES
		"src/flags.ixx"
		"src/logger.ixx"
		"src/timer.ixx"
		"src/geometry.ixx"
		"src/geometry/-basic_operations.ixx"
		"src/geometry/-composition.ixx"
		"src/geometry/-shapes.ixx"
		"src/main_templates/singleprocess.ixx"
		"src/main_templates/singleprocess/-common.ixx"
		"src/main_templates/singleprocess/-light.ixx"
		"src/main_templates/singleprocess/-heavy.ixx"
		"src/main_templates/multiprocess.ixx"
		"src/main_templates/dispatch.ixx"
		"src/os.ixx"
		"src/os/linux.ixx"
		"src/os/linux/-common.ixx"
		"src/os/linux/-unique_handle.ixx"
		"src/os/winapi.ixx"
		"src/os/winapi/-common.ixx"
		"src/os/winapi/-pipe.ixx"
		"src/os/winapi/-process.ixx"
		"src/os/winapi/-unique_handle.ixx"
		"src/concurrent/thread_pool.ixx"
		"src/concurrent/thread_pool/-pool.ixx"
		"src/concurrent/thread_pool/-worker.ixx"
		"src/concurrent/type_suppliers.ixx"
)
