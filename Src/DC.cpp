#include "../Private/DCImpl.h"

void S1Data::DCXmlExporter::ExportElement(DCElement86* element, const std::filesystem::path& dst)
{
  if (!element || !Storage)
  {
    return;
  }
  pugi::xml_document doc;
  Impl->CreateXmlNode(element, doc);
  std::filesystem::path p(dst);
  doc.save_file(p.replace_extension("xml").wstring().c_str());
}

void S1Data::DCXmlExporter::ExportElement(DCElement64* element, const std::filesystem::path& dst)
{
  if (!element || !Storage)
  {
    return;
  }
  pugi::xml_document doc;
  Impl->CreateXmlNode(element, doc);
  std::filesystem::path p(dst);
  doc.save_file(p.replace_extension("xml").wstring().c_str());
}

void S1Data::DCJsonExporter::ExportElement(DCElement86* element, const std::filesystem::path& dst)
{
  if (!element || !Storage)
  {
    return;
  }

  rapidjson::StringBuffer sb;
  rapidjson::PrettyWriter writer(sb);
  writer.SetIndent(' ', 2);

  Impl->CreateJsonNode(element, &writer);

  std::filesystem::path p(dst);
  FWriteStream s(p.replace_extension("json").wstring());
  s.SerializeBytes((void*)sb.GetString(), (FILE_OFFSET)sb.GetSize());
}

void S1Data::DCJsonExporter::ExportElement(DCElement64* element, const std::filesystem::path& dst)
{
  if (!element || !Storage)
  {
    return;
  }

  rapidjson::StringBuffer sb;
  rapidjson::PrettyWriter writer(sb);
  writer.SetIndent(' ', 2);

  Impl->CreateJsonNode(element, &writer);

  std::filesystem::path p(dst);
  FWriteStream s(p.replace_extension("json").wstring());
  s.SerializeBytes((void*)sb.GetString(), (FILE_OFFSET)sb.GetSize());
}

S1Data::DCExporter::DCExporter(DCInterface* storage)
  : Storage(storage)
  , Impl(new DCExporterImpl(storage))
{}

S1Data::DCExporter::~DCExporter()
{
  delete Impl;
}