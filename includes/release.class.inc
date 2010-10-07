// A general Drupal Project release with its meta data
class Release extends FeedEntryContainer{
  public function __construct($info) {
    parent::__construct($info, $terms);
  }
  
  protected function validate_terms($terms)  {
    $req = array ( 'Release type' ); 
    if(!$this->array_keys_exists($req, $terms)) {
      throw new Exception('Not all required terms set from release. Required: ('.join(',',$req).')');
    }
  }
  protected function validate_info($info) {
    $req = array ( 'name', 'version', 'tag', 'version_major', 'version_extra', 
                  'status', 'release_link', 'download_link', 'date', 'mdhash', 'filesize' );
    if(!$this->array_keys_exists($req, $info)) {
      throw new Exception('Not all required infos set from release. Required: ('.join(',',$req).')');
    }
  }
}
