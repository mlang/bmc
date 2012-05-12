// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <ctime>
#include <iostream>
#include <memory>

namespace music { namespace musicxml {

class xml_string
{
  XMLCh *transcoded;
public:
  xml_string(char const * const string)
  : transcoded(XERCES_CPP_NAMESPACE::XMLString::transcode(string))
  {}
  ~xml_string() { XERCES_CPP_NAMESPACE::XMLString::release(&transcoded); }

  operator XMLCh const *() const { return transcoded; }
};

class ostream_format_target: public XERCES_CPP_NAMESPACE::XMLFormatTarget
{
  std::ostream& stream;
public:
  ostream_format_target(std::ostream &stream): stream(stream) {}

  virtual void writeChars( XMLByte const * const buffer, XMLSize_t const size
                         , xercesc::XMLFormatter* const
                         )
  {
    if (not stream.fail())
      stream.write(reinterpret_cast<char const *>(buffer),
                   static_cast<std::streamsize>(size));
  }

  virtual void flush() { if (not stream.fail()) stream.flush(); }
};

/**
 * \brief A deleter that uses the <code>release</code> member function to destroy
 * an object.
 *
 * Xerces DOM objects have a reference counting mechanism where calling
 * <code>release</code> on them decrements the reference counter.
 */
struct dom_deleter
{
  template <class T>
  void operator()(T* ptr)
  { if (ptr) ptr->release(); }
};

typedef std::unique_ptr<xercesc::DOMDocument, dom_deleter>
        unique_dom_document_ptr;
typedef std::unique_ptr<xercesc::DOMLSSerializer, dom_deleter>
        unique_dom_ls_serializer_ptr;
typedef std::unique_ptr<xercesc::DOMLSOutput, dom_deleter>
        unique_dom_ls_output_ptr;

class document
{
  unique_dom_document_ptr dom_document;
  XERCES_CPP_NAMESPACE::DOMElement *score_partwise, *identification, *part_list;
public:
  document()
  : dom_document(create_dom_document())
  , score_partwise(dom_document->getDocumentElement())
  , identification(create_identification())
  , part_list(dom_document->createElement(xml_string("part-list")))
  { initialize_empty_document(); }

  ~document() { score_partwise = identification = part_list = nullptr; }

private:
  static XERCES_CPP_NAMESPACE::DOMDocument *create_dom_document()
  {
    XERCES_CPP_NAMESPACE::DOMImplementation
    *dom(XERCES_CPP_NAMESPACE::
         DOMImplementationRegistry::getDOMImplementation(xml_string("Core")));
    if (dom) {
      xml_string
      score_type("score-partwise"),
      dtd_public("-//Recordare//DTD MusicXML 3.0 Partwise//EN"),
      dtd_url("http://www.musicxml.org/dtds/partwise.dtd");

      return dom->createDocument(nullptr, score_type,
                                 dom->createDocumentType(score_type,
                                                         dtd_public, dtd_url));
    } else {
      return nullptr;
    }
  }

  void initialize_empty_document()
  {
    score_partwise->setAttribute(xml_string("version"), xml_string("3.0"));
    score_partwise->appendChild(identification);
    score_partwise->appendChild(part_list);
  }

  XERCES_CPP_NAMESPACE::DOMElement *create_identification() const
  {
    XERCES_CPP_NAMESPACE::DOMElement
    *element(dom_document->createElement(xml_string("identification"))),
    *encoding(dom_document->createElement(xml_string("encoding"))),
    *software(dom_document->createElement(xml_string("software")));

    encoding->appendChild(create_encoding_date());
    software->appendChild(dom_document->createTextNode(xml_string("Braille Music Compiler")));
    encoding->appendChild(software);
    element->appendChild(encoding);
    return element;
  }

  XERCES_CPP_NAMESPACE::DOMElement *create_encoding_date() const
  {
    XERCES_CPP_NAMESPACE::DOMElement
    *element = dom_document->createElement(xml_string("encoding-date"));
    char date_string[11];
    std::time_t current_time = std::time(nullptr);
    std::strftime(date_string, sizeof(date_string), "%Y-%m-%d",
                  std::localtime(&current_time));
    element->appendChild(dom_document->createTextNode(xml_string(date_string)));
    return element;
  }

public:
  bool write_to_stream(std::ostream& stream) const
  {
    XERCES_CPP_NAMESPACE_USE

    try {
      DOMImplementationLS
      *ls = DOMImplementationRegistry::getDOMImplementation(xml_string("ls"));
      unique_dom_ls_serializer_ptr serializer(ls->createLSSerializer());
      DOMConfiguration *configuration = serializer->getDomConfig();

      if (configuration->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
        configuration->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);

      unique_dom_ls_output_ptr output(ls->createLSOutput());
      ostream_format_target format_target(stream);
      output->setByteStream(&format_target);

      return serializer->write(dom_document.get(), output.get());
    } catch (const OutOfMemoryException&) {
      std::cerr << "OutOfMemoryException" << std::endl;
    } catch (XMLException& e) {
      std::cerr << "An error occurred during creation of output transcoder. Msg is:"
		<< std::endl
		<< e.getMessage() << std::endl;
    }
    return false;
  }
  friend std::ostream& operator<< (std::ostream& stream, document const& doc)
  {
    doc.write_to_stream(stream);
    return stream;
  }
};

}}

#include <xercesc/util/PlatformUtils.hpp>

int main()
{
  XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();
  {
    music::musicxml::document musicxml;
    std::cout << musicxml;
  }
  XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate();
  return EXIT_SUCCESS;
}
