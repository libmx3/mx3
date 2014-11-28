#import "MXSampleDataTableViewController.h"
#import "gen/MX3UserListVmCell.h"

NSString *const CellIdentifier = @"MX3Cell";

@interface MXSampleDataTableViewController ()

@property (nonatomic) id <MX3Api> api;
@property (nonatomic) id <MX3UserListVmHandle> handle;
@property (nonatomic) id <MX3UserListVm> viewModel;

@end

@implementation MXSampleDataTableViewController

- (instancetype) initWithApi:(id<MX3Api>)api {
    self = [super initWithStyle:UITableViewStylePlain];
    if (self) {
        self.api = api;
        self.handle = [api observerUserList];
        self.viewModel = nil;
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = [self.api getUsername];
    [self.tableView registerClass:UITableViewCell.class forCellReuseIdentifier:CellIdentifier];
}

- (void)viewWillAppear:(BOOL)animated {
    // todo(piet) what is the proper 'weak-ref' style here
    [self.handle start:self];
}

- (void)viewWillDisappear:(BOOL)animated {
    [self.handle stop];
}

- (void)onUpdate:(id <MX3UserListVm>)newData {
    self.viewModel = newData;
    // todo(kabbes) this would be awesome to automatically perform animations
    [self.tableView reloadData];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.viewModel != nil ? [self.viewModel count] : 0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier
                                                            forIndexPath:indexPath];

    MX3UserListVmCell * cellData = [self.viewModel get:indexPath.row];
    cell.textLabel.text = [cellData name];
    cell.detailTextLabel.text = @"If you manage to get the deps right";
    return cell;
}

@end
