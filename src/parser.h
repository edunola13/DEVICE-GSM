String parseProperty(String* json, String property, uint8_t maxLength);
String propertyToJson(String name, String value, bool comillas, bool ultimo);

String parseProperty(String* json, String property, uint8_t maxLength){
  String val= "\"" + property + "\":";
  int i= json->indexOf(val);
  if(i != -1){
    String subStr= json->substring(i + val.length() + 1, i + val.length() + 1 + maxLength + 1);
    i= subStr.indexOf("\"");
    subStr= subStr.substring(0, i);
    return subStr;
  }else{
    return "null";
  }
}

String propertyToJson(String name, String value, bool comillas, bool ultimo) {
	String val= "\"" + name + "\": ";
	if(comillas){
	  val+= "\"" + value + "\"";
	}else{
	  val+= value;
	}
	if(!ultimo){
	  val+=",";
	}
	return val;
}
