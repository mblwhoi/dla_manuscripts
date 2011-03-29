<div class="collection_gallery_row clearfix">
  <div class="image">
     <!--
    <img src="/finding_aid_resources/images/<?php print trim($row->node_data_field_ead_collection_id_field_ead_collection_image_value);?>.jpg"/>
     -->
    <?php print trim($fields['field_ead_collection_image_fid']->content);?>
  </div>

  <div class="content">
    <div class="head">
     <?php print $fields['title']->content;?>
    </div>

    <div class="abstract">
      <?php print $fields['field_ead_abstract_value']->content;?>
    </div>

    <div class="dates">
     <div class="head">
       Dates:
     </div>
     <div class="content">
       <?php print $fields['field_ead_dates_value']->content;?>
     </div>
    </div>

  </div>

</div>



