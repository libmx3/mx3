#import "MXSampleDataTableViewController.h"
#import "MX3Api+iOS.h"

NSString *const CellIdentifier = @"MX3Cell";

@interface MXSampleDataTableViewController ()

@end

@implementation MXSampleDataTableViewController

- (void)viewDidLoad {
    [super viewDidLoad];
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
    return 1;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier
                                                            forIndexPath:indexPath];

    cell.textLabel.text = @"MX3 rocks";
    cell.detailTextLabel.text = @"If you manage to get the deps right";

    return cell;
}

@end
