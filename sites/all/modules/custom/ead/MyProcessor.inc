<?php
/**
 * Processes My Feed
 */
class MyProcessor extends FeedsProcessor {

/**
 * Process the result of the parser or previous processors.
 *
 * @param FeedsImportBatch $batch
 *   The current feed import data passed in from the parsing stage.
 * @param FeedsSource $source
 *   Source information about this import.
 *
 * @return
 *   FEEDS_BATCH_COMPLETE if all items have been processed, a float between 0
 *   and 0.99* indicating progress otherwise.
 */
    function process(FeedsImportBatch $batch, FeedsSource $source) {
        module_load_include('inc', 'node', 'node.pages');  // new for Drupal 6

        // create a collection node
        $node = new stdClass();

        $batch->created++;

        // Populate and prepare node object.
        $node->type = 'ead_collection';
        $node->title = 'Borgalo Test';
        $node->body = 'Borgalo Test body carba carba';
        $node->created = FEEDS_REQUEST_TIME;
        static $included;
        if (!$included) {
            module_load_include('inc', 'node', 'node.pages');
            $included = TRUE;
        }
        node_object_prepare($node);

        // Populate properties that are set by node_object_prepare().
        $node->log = 'Created/updated by Feeds [[TEST!!]]';
        $node->uid = 0;


        // Save the node.
        node_save($node);
        
        return FEEDS_BATCH_COMPLETE;

    }


    /**
     * Remove all stored results or stored results up to a certain time for this
     * configuration/this source.
     *
     * @param FeedsBatch $batch
     *   A FeedsBatch object for tracking information such as how many
     *   items have been deleted total between page loads.
     * @param FeedsSource $source
     *   Source information for this expiry. Implementers should only delete items
     *   pertaining to this source. The preferred way of determining whether an
     *   item pertains to a certain souce is by using $source->feed_nid. It is the
     *   processor's responsibility to store the feed_nid of an imported item in
     *   the processing stage.
     *
     * @return
     *   FEEDS_BATCH_COMPLETE if all items have been processed, a float between 0
     *   and 0.99* indicating progress otherwise.
     */
    function clear(FeedsBatch $batch, FeedsSource $source) {
        return FEEDS_BATCH_COMPLETE;
    }
}
?>