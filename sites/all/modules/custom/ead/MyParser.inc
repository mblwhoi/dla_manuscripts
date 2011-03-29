<?php
/**
* Parses My Feed
*/
class MyParser extends FeedsParser {

  /**
  * Parses a raw string and populates FeedsImportBatch object from it.
  */
  public function parse(FeedsImportBatch $batch, FeedsSource $source) {
    // Get the file's content.
    $string = $batch->getRaw();

    // Parse it...

    // The parsed result should be an array of arrays of field name => value.
    // This is an example of such an array:
    $items = array();
    $items[] = array(
      'guid' => 'MyGuid1',
      'title' => 'My Title',
    );
    $items[] = array(
      'guid' => 'MyGuid2',
      'title' => 'My Other Title',
    );

    // Populate the FeedsImportBatch object with the parsed results.
    $batch->setTitle('Example title');
    $batch->setItems($items);
  }

  public function getMappingSources() {
    return array(
      'guid' => array(
        'name' => t('GUID'),
        'description' => t('Unique ID.'),
      ),
      'title' => array(
        'name' => t('Title'),
        'description' => t('My Title.'),
      ),
    );
  }

}
?>