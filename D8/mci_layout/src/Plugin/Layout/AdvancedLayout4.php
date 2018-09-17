<?php

namespace Drupal\mci_layout\Plugin\Layout;

use Drupal\Core\Layout\LayoutDefault;

/**
* A very advanced custom layout.
*
* @Layout(
*   id = "advanced_layout_4",
*   label = @Translation("Advanced Layout 4"),
*   category = @Translation("My Layouts"),
*   template = "templates/advanced-layout-4",
*   library = "mci_layout/advanced-layout-library",
*   regions = {
*     "main" = {
*       "label" = @Translation("Main content"),
*     }
*   }
* )
*/
class AdvancedLayout4 extends LayoutDefault {
// Override any methods you'd like to customize here!
}
