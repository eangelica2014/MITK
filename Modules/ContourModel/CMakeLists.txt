MITK_CREATE_MODULE(
  INCLUDE_DIRS Algorithms DataManagement IO Rendering
  DEPENDS MitkCore MitkSceneSerializationBase MitkLegacyGL MitkOverlays
  PACKAGE_DEPENDS ITK|ITKReview
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
