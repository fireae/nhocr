-- Quick and dirty script of OCRopus(0.3)-NHocr bridge
--   by Hideaki Goto on May 15, 2009
--
-- Example of using NHocr as a line recognizer
-- with OCRopus layout analysis.
--
-- Installation:
--   Save this Lua script as
--    ${OCROPUS_INSTALLDIR}/share/ocropus/scripts/rec-nhocr.lua
--
-- Usage:
--    $ OCROSCRIPTS=${OCROPUS_INSTALLDIR}/share/ocropus/scripts \
--      ocroscript rec-nhocr input_image_file > output.txt
--
-- Notes:
--   This script produces a temporary file line.pgm in the current directory.
--   Please be sure to run ocroscript in a user-writable directory.


import_all(ocr)
import_all(iulib)

if #arg < 1 then
    print("Usage: ocroscript rec-nhocr input_image_file > output.txt ")
    os.exit(1)
end

pages = Pages()
pages:parseSpec(arg[1])

segmenter = make_SegmentPageByRAST()
page_image = bytearray()
page_segmentation = intarray()
line_image = bytearray()

while pages:nextPage() do
   pages:getBinary(page_image)
   segmenter:segment(page_segmentation,page_image)
   regions = RegionExtractor()
   regions:setPageLines(page_segmentation)
   for i = 1,regions:length()-1 do
      regions:extract(line_image,page_image,i,1)
      write_image_gray("line.pgm", line_image)
      os.execute("/opt/nhocr/bin/nhocr -line -o - line.pgm ; rm line.pgm")
   end
end
