#import "MXSampleDataTableViewController.h"
#import "MX3Api+iOS.h"

NSString *const CellIdentifier = @"MX3Cell";

@interface MXSampleDataTableViewController () {
    MX3Snapshot * __snapshot;
}

@end

@implementation MXSampleDataTableViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    __snapshot = [[MX3Api sharedAPI] launches];
    [self setupNavigationBar];
    [self registerCells];
}

- (void)setupNavigationBar {
    self.title = [[MX3Api sharedAPI] username];
}

- (void)registerCells {
    [self.tableView registerClass:UITableViewCell.class forCellReuseIdentifier:CellIdentifier];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [__snapshot count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier
                                                            forIndexPath:indexPath];

    cell.textLabel.text = [__snapshot rowAtIndex: indexPath.row];
    cell.detailTextLabel.text = @"If you manage to get the deps right";

    return cell;
}

@end
