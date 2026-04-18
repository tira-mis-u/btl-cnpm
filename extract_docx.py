import sys
import zipfile
import xml.etree.ElementTree as ET
import io

def extract_text_from_docx(docx_path):
    document = zipfile.ZipFile(docx_path)
    xml_content = document.read('word/document.xml')
    document.close()
    tree = ET.XML(xml_content)
    
    WORD_NAMESPACE = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
    PARA = WORD_NAMESPACE + 'p'
    TEXT = WORD_NAMESPACE + 't'
    TABLE = WORD_NAMESPACE + 'tbl'
    ROW = WORD_NAMESPACE + 'tr'
    CELL = WORD_NAMESPACE + 'tc'

    text = []
    for element in tree.iter():
        if element.tag == PARA:
            texts = [node.text for node in element.iter(TEXT) if node.text]
            if texts:
                text.append(''.join(texts))
        elif element.tag == TABLE:
            text.append('\n--- Table Start ---')
            for row in element.iter(ROW):
                row_text = []
                for cell in row.iter(CELL):
                    cell_texts = [node.text for node in cell.iter(TEXT) if node.text]
                    row_text.append(' | '.join(cell_texts))
                text.append(' || '.join(row_text))
            text.append('--- Table End ---\n')
    
    return '\n'.join(text)

if __name__ == '__main__':
    with io.open('extracted.txt', 'w', encoding='utf-8') as f:
        f.write(extract_text_from_docx(sys.argv[1]))
