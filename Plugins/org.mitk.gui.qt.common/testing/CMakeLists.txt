project(org_mitk_gui_qt_common_tests)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE org_mitk_gui_qt_common_tests_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src src/api
  MODULE_DEPENDS MitkQtWidgets
  TEST_PLUGIN
)

target_link_libraries(${PROJECT_NAME} optimized CppUnit debug CppUnitd)

MACRO_TEST_UIPLUGIN(${MITK_DEFAULT_SUBPROJECTS})

