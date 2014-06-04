#import "MXSampleDataTableViewController.h"
#import "MX3Api+iOS.h"
#import "MX3GithubAPI.h"

NSString *const CellIdentifier = @"MX3Cell";

@interface MXSampleDataTableViewController ()

@property (nonatomic) MX3QueryResult *result;

@end

@implementation MXSampleDataTableViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.result = [[MX3Api sharedAPI] githubUsers];
    [self.result listenToChanges:^{
      [self.tableView reloadData];
    }];
    [self setupNavigationBar];
    [self registerCells];

    [MX3GithubAPI getUsersWithSuccess:^(id JSON) {
        NSLog(@"%@", JSON);
    } failure:^(NSError *error) {
        NSLog(@"%@", error);
    }];
}

- (void)setupNavigationBar {
    self.title = [[MX3Api sharedAPI] username];
}

- (void)registerCells {
    [self.tableView registerClass:UITableViewCell.class forCellReuseIdentifier:CellIdentifier];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.result.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier
                                                            forIndexPath:indexPath];

    cell.textLabel.text = [self.result rowAtIndex:(NSUInteger)indexPath.row];
    cell.detailTextLabel.text = @"If you manage to get the deps right";

    return cell;
}

@end
