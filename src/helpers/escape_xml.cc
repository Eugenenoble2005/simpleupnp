
#include "escape_xml.h"

std::string EscapeXML(std::string & xml){
  std::string output;
  output.reserve(xml.size());
  for( char c : xml){
    switch(c){
      case '<':
        output.append("&lt;");
        break;
      case '>':
        output.append("&gt;");
        break;
      case '&':
        output.append("&amp;");
        break;
      case '\"':
        output.append("&quot;");
        break;
      case '\'':
        output.append("&apos;");
        break;
      default:
        output.push_back(c);
        break;
    }
  }
  return output;
}
